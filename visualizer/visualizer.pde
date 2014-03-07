import processing.net.*;

/*
 * Node : heap object under GC
 */
final byte NOP    = 0;
final byte MARKED = 1;
final byte ROOT   = 2;

class Node {
  // parameter as heap object
  public int  typeid;
  public long addr;
  public ArrayList<Node> ref;
  public byte status;

  // parameter as visualied node
  public float r;
  public float x,  y;
  public float dx, dy;

  public Node(int typeid, long addr) {
    this.typeid = typeid;
    this.addr   = addr;
    this.ref    = new ArrayList<Node>();
    this.status = NOP;
    this.r      = 5;
    this.x      = random(this.r, width-this.r);
    this.y      = random(this.r, height-this.r);
    this.dx     = 0;
    this.dy     = 0;
  }

  public String toString() {
    return "{Node addr: " + addr + " x: " + x + " y: " + y + " dx: " + dx + " dy: " + dy + "}";
  }
}

/*
 * addrTable : manage nodes
 */
HashMap<Long, Node> addrTable = new  HashMap<Long, Node>();

/*
 * Node movement simulation
 */

final float S = 0.2; // spring constant
final float C = 10; // coulomb's constant
final float len = 10;

void updateNodes() {
  for (Node node : addrTable.values()) {
    for (Node hoge : addrTable.values()) {
      if (node == hoge) continue;

      float dst = dist(node.x, node.y, hoge.x, hoge.y);
      float dx  = hoge.x - node.x;
      float dy  = hoge.y - node.y;

      if (Math.abs(dx) < 1.0 && Math.abs(dy) < 1.0) continue;

      node.dx -= C * dx/(dst*dst*dst);
      node.dy -= C * dy/(dst*dst*dst);
    }

    for (Node ref : node.ref) {
      float dst = dist(node.x, node.y, ref.x, ref.y);
      float dx  = ref.x - node.x;
      float dy  = ref.y - node.y;

      if (Math.abs(dx) < 1.0 && Math.abs(dy) < 1.0) continue;

      node.dx += S * (dst-len) * dx/dst;
      node.dy += S * (dst-len) * dy/dst;
      ref.dx  -= S * (dst-len) * dx/dst;
      ref.dy  -= S * (dst-len) * dy/dst;
    }
  }

  for (Node node : addrTable.values()) {
    if (Float.isNaN(node.dx)) node.dx = 0;
    if (Float.isNaN(node.dy)) node.dy = 0;

    float dx1 = -node.x+node.r-0.1;
    float dx2 = width-node.x-node.r+0.1;
    float dy1 = -node.y+node.r-0.1;
    float dy2 = height-node.y-node.r+0.1;
    node.dx -= C * (1/dx1 + 1/dx2);
    node.dy -= C * (1/dy1 + 1/dy2);
    node.dx = constrain(node.dx, -20, 20);
    node.dy = constrain(node.dy, -20, 20);

    node.x += node.dx;
    node.y += node.dy;

    node.dx *= 0.5;
    node.dy *= 0.5;

    node.x = constrain(node.x, node.r, width-node.r);
    node.y = constrain(node.y, node.r, height-node.r);
  }
}

void drawNodes() {
  for (Node node : addrTable.values()) {
    noStroke();
    if (node.status == NOP)
      fill(130);
    else
      fill(50);
    ellipse(node.x, node.y, node.r, node.r);

    for (Node ref : node.ref) {
      stroke(130, 50);
      line(node.x, node.y, ref.x, ref.y);
    }
  }
}

/*
 * GC operation
 */

void opAlloc(int typeid, long addr) {
  Node node = new Node(typeid, addr);
  
  addrTable.put(addr, node);
}

void opRef(long from, long to) {
  Node obj = addrTable.get(from);
  Node ref = addrTable.get(to);

  obj.ref.add(ref);
}

void opDeref(long from, long to) {
  Node obj = addrTable.get(from);
  Node ref = addrTable.get(to);

  obj.ref.remove(ref);
}

void opMark(long addr, byte status) {
  Node obj = addrTable.get(addr);
  
  obj.status = status;
}

void opSweep() {
  HashMap<Long, Node> newTable = new HashMap<Long, Node>();
  for(Node node : addrTable.values()) {
    if (node.status != NOP) {
      node.status = NOP;
      newTable.put(node.addr, node);
    }
  }
  addrTable = newTable;
}

/*
 * socket interface
 */

Client socket;

final byte ALLOC = 1;
final byte REF   = 2;
final byte DEREF = 3;
final byte MARK  = 4;
final byte SWEEP = 5;

byte receiveByte() {
  return (byte)socket.readChar();
}

int receiveInt() {
  int val = 0, j = 1;;
  for (int i = 0; i < 4; i++, j*=256)
    val = val + (int)socket.readChar()*j;
  return val;
}

long receiveLong() {
  long val = 0, j = 1;
  for (int i = 0; i < 8; i++, j*=256)
    val = val + (long)socket.readChar()*j;
  return val;
}

void receive() {
  if (socket.available() <= 0)
    return;

  byte opcode = receiveByte();
  
  switch (opcode) {
  case ALLOC: {
    int  typeid = receiveInt();
    long addr   = receiveLong();
    // System.out.printf("ALLOC {type: %d, addr: %x}\n", typeid, addr);
    opAlloc(typeid, addr);
  } break;

  case REF: {
    long from = receiveLong();
    long to   = receiveLong();
    // System.out.printf("REF   {from: %x, to: %x}\n", from, to);
    opRef(from, to);
  } break;

  case DEREF: {
    long from = receiveLong();
    long to   = receiveLong();
    // System.out.printf("DEREF {from: %x, to: %x}\n", from, to);
    opDeref(from, to);
  } break;

  case MARK: {
    long addr   = receiveLong();
    byte status = receiveByte();
    // System.out.printf("MARK  {addr: %x, stat: %d}\n", addr, status);
    opMark(addr, status);
  } break;

  case SWEEP:
    // System.out.printf("SWEEP\n");
    opSweep();
    break;

  default: 
    println("ERROR: wrong opcode");
    exit();
  }
}

/*
 * main routine
 */

void setupClient() {
  socket = new Client(this, "127.0.0.1", 5001);
}

void setup() {
  size(900, 600);

  setupClient();
}

void draw() {
  receive();

  updateNodes();
  background(0);
  noStroke();
  drawNodes();
}
