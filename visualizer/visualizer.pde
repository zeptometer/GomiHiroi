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
}

/*
 * addrTable : manage nodes
 */
HashMap<Long, Node> addrTable = new  HashMap<Long, Node>();

/*
 * Node movement simulation
 */

final float S = 0.5; // spring constant
final float C = 0.2; // coulomb's constant

void updateNodes() {
  for (Node node : addrTable.values()) {
    for (Node hoge : addrTable.values()) {
      float dst = dist(node.x, node.y, hoge.x, hoge.y);
      float dx  = hoge.x - node.x;
      float dy  = hoge.y - node.y;

      node.dx -= C * dx/(dst*dst);
      node.dy -= C * dy/(dst*dst);
    }

    for (Node ref : node.ref) {
      float dst = dist(node.x, node.y, ref.x, ref.y);
      float dx  = ref.x - node.x;
      float dy  = ref.y - node.y;

      node.dx += S * dx/(dst*dst);
      node.dy += S * dy/(dst*dst);
      ref.dx -= S * dx/(dst*dst);
      ref.dy -= S * dy/(dst*dst);
    }
  }

  for (Node node : addrTable.values()) {
    node.x += node.dx;
    node.y += node.dy;

    node.x = constrain(node.r, node.x, width-node.r);
    node.y = constrain(node.r, node.y, height-node.r);
  }
}

void drawNodes() {
  for (Node node : addrTable.values()) {
    noStroke();
    fill(130);
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

final byte ALLOC = 0;
final byte REF   = 1;
final byte DEREF = 2;
final byte MARK  = 3;
final byte SWEEP = 4;

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
    System.out.printf("ALLOC {type: %d, addr: %x}\n", typeid, addr);
    opAlloc(typeid, addr);
  } break;

  case REF: {
    long from = receiveLong();
    long to   = receiveLong();
    System.out.printf("REF {from: %x, to: %x}\n", from, to);
    opRef(from, to);
  } break;

  case DEREF: {
    long from = receiveLong();
    long to   = receiveLong();
    System.out.printf("DEREF {from: %x, to: %x}\n", from, to);
    opDeref(from, to);
  } break;

  case MARK: {
    long addr   = receiveLong();
    byte status = receiveByte();
    System.out.printf("MARK {addr: %x, stat: %d}\n", addr, status);
    opMark(addr, status);
  } break;

  case SWEEP:
    System.out.printf("SWEEP\n");
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

void setup() {
  size(900, 600);
  socket = new Client(this, "127.0.0.1", 5001);
}

void draw() {
  receive();

  updateNodes();
  background(0);
  noStroke();
  drawNodes();
}
