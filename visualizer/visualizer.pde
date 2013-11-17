import processing.net.*;

final int CONS    = 0;
final int CLOSURE = 1;
final int ENV     = 2;

class MemObj {
  public int type;
  public int addr;
  public int ref1, ref2;
  public boolean mark;

  public MemObj(int type, int addr) {
    this.type = type;
    ref1 = 0; ref2 = 0;
    mark = false;
  }
}

class MemIndex {
  public int type;
  public int idx;

  public MemIndex(int type, int idx) {
    this.type = type;
    this.idx  = idx;
  }
}

final int MAX_N_OBJ = 1000000;
HashMap<Integer, MemIndex> table = new HashMap<Integer, MemIndex>();
ArrayList<MemObj> cons    = new ArrayList<MemObj>(MAX_N_OBJ);
ArrayList<MemObj> closure = new ArrayList<MemObj>(MAX_N_OBJ);
ArrayList<MemObj> env     = new ArrayList<MemObj>(MAX_N_OBJ);

MemObj getMemObj(int addr) {
  MemIndex mi = table.get(addr);
  if (mi == null) {
    System.out.printf("ERROR : addr %x not found\n", addr);
    exit();
  }
  switch (mi.type) {
  case CONS:
    return cons.get(mi.idx);
  case CLOSURE:
    return closure.get(mi.idx);
  case ENV:
    return env.get(mi.idx);
  default:
    println("WARNING : wrong type");
    return null;
  }
}


void alloc (String type, int addr) {
  if (type.equals("CONS")) {
    table.put(addr, new MemIndex(CONS, cons.size()));
    cons.add(new MemObj(CONS, addr));

  } else if (type.equals("CLOSURE")) {
    table.put(addr, new MemIndex(CLOSURE, closure.size()));
    closure.add(new MemObj(CLOSURE, addr));

  } else if (type.equals("ENV")) {
    table.put(addr, new MemIndex(ENV, env.size()));
    env.add(new MemObj(ENV, addr));
  } else  {
    System.out.printf("ERROR : type%stype : %x\n", type, addr);
    exit();
  }

  System.out.printf("ALLOC : %s : %x\n", type, addr);
}

void ref (int from, int to) {
  MemObj obj = getMemObj(from);
  if (obj.ref1 == 0) {
    obj.ref1 = to;
  } else if (obj.ref2 == 0) {
    obj.ref2 = to;
  } else {
    println("WARNING: reference is full!!!");
  }
  System.out.printf("REF : %x : %x\n", from, to);
}

void deref (int from, int to) {
  MemObj obj = getMemObj(from);
  if (obj.ref1 == to) {
    obj.ref1 = obj.ref2;
    obj.ref2 = 0;
  } else if (obj.ref2 == 0) {
    obj.ref2 = to;
  } else {
    println("WARNING: not refering!!");
  }
  System.out.printf("DEREF : %x : %x\n", from, to);
}

void mark (int addr) {
  getMemObj(addr).mark = true;
  System.out.printf("MARK : %x\n", addr);
}

void sweep () {
  println("SWEEP");
}

Client myClient;

void receive() {
  String   op;
  String[] ops;
  if (myClient.available() > 0) {
    op = myClient.readStringUntil(0);
    op = op.substring(0, op.length()-1);
    ops = op.split(" : ");
    if (ops[0].equals("ALLOC")) {
      String objtype = ops[1];
      int    addr    = Integer.parseInt(ops[2].substring(2), 16);
      alloc(objtype, addr);
    } else if (ops[0].equals("REF")) {
      int from = Integer.parseInt(ops[1].substring(2), 16);
      int to   = Integer.parseInt(ops[2].substring(2), 16);
      ref(from, to);
    } else if (ops[0].equals("DEREF")) {
      int from = Integer.parseInt(ops[1].substring(2), 16);
      int to   = Integer.parseInt(ops[2].substring(2), 16);
      deref(from, to);
    } else if (ops[0].equals("MARK")) {
      int addr = Integer.parseInt(ops[1].substring(2), 16);
      mark(addr);
    } else if (ops[0].equals("SWEEP")) {
      sweep();
    } else {
      println("WARNING : unknown opcode");
    }
  }
}

void setup() {
  size(600, 900);
  myClient = new Client(this, "127.0.0.1", 5001);
}

void draw() {
  receive();
}
