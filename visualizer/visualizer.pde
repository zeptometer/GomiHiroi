import processing.net.*;

final long ENV     = 0;
final long CLOSURE = 1;
final long CONS    = 2;


class MemObj {
  public long type;
  public long addr;
  public long idx;
  public ArrayList<MemObj> ref;
  public boolean mark;

  public MemObj(long type, long addr, long idx) {
    this.type = type;
    this.addr = addr;
    this.idx  = idx;
    this.ref  = new ArrayList<MemObj>();
    this.mark = false;
  }
}

final int MAX_N_OBJ = 1000000;
HashMap<Long, MemObj> table = new HashMap<Long, MemObj>();
ArrayList<MemObj> cons    = new ArrayList<MemObj>(MAX_N_OBJ);
ArrayList<MemObj> closure = new ArrayList<MemObj>(MAX_N_OBJ);
ArrayList<MemObj> env     = new ArrayList<MemObj>(MAX_N_OBJ);

void alloc (String type, long addr) {
  if (type.equals("CONS")) {
    long idx = cons.size();
    MemObj obj = new MemObj(CONS, addr, idx);
    table.put(addr, obj);
    cons.add(obj);

  } else if (type.equals("CLOSURE")) {
    long idx = closure.size();
    MemObj obj = new MemObj(CLOSURE, addr, idx);
    table.put(addr, obj);
    closure.add(obj);

  } else if (type.equals("ENV")) {
    long idx = env.size();
    MemObj obj = new MemObj(ENV, addr, idx);
    table.put(addr, obj);
    env.add(obj);

  } else  {
    System.out.printf("ERROR : type%stype : %x\n", type, addr);
    exit();
  }

  System.out.printf("ALLOC : %s : %x\n", type, addr);
}

void ref (long from, long to) {
  MemObj fromobj = table.get(from);
  MemObj toobj   = table.get(to);

  fromobj.ref.add(toobj);
  System.out.printf("REF : %x : %x\n", from, to);
}

void deref (long from, long to) {
  MemObj fromobj = table.get(from);
  MemObj toobj   = table.get(to);
  fromobj.ref.remove(toobj);
  System.out.printf("DEREF : %x : %x\n", from, to);
}

void mark (long addr) {
  table.get(addr).mark = true;
  System.out.printf("MARK : %x\n", addr);
}

void sweep () {
  java.util.Iterator itr = table.values().iterator();
  while (itr.hasNext()) {
    MemObj mo = (MemObj)itr.next();
    if (!mo.mark)
      itr.remove();
  }

  ArrayList<MemObj> newcons = new ArrayList<MemObj>(MAX_N_OBJ);
  for (MemObj mo : cons)
    if (mo.mark) {
      mo.mark = false;
      mo.idx = newcons.size();
      newcons.add(mo);
    }
  cons = newcons;

  ArrayList<MemObj> newclosure = new ArrayList<MemObj>(MAX_N_OBJ);
  for (MemObj mo : closure)
    if (mo.mark) {
      mo.mark = false;
      mo.idx = newclosure.size();
      newclosure.add(mo);
    }
  closure = newclosure;

  ArrayList<MemObj> newenv = new ArrayList<MemObj>(MAX_N_OBJ);
  for (MemObj mo : env)
    if (mo.mark) {
      mo.mark = false;
      mo.idx = newenv.size();
      newenv.add(mo);
    }
  env = newenv;
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
      long    addr    = Long.parseLong(ops[2].substring(2), 16);
      alloc(objtype, addr);
    } else if (ops[0].equals("REF")) {
      long from = Long.parseLong(ops[1].substring(2), 16);
      long to   = Long.parseLong(ops[2].substring(2), 16);
      ref(from, to);
    } else if (ops[0].equals("DEREF")) {
      long from = Long.parseLong(ops[1].substring(2), 16);
      long to   = Long.parseLong(ops[2].substring(2), 16);
      deref(from, to);
    } else if (ops[0].equals("MARK")) {
      long addr = Long.parseLong(ops[1].substring(2), 16);
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
  try {
    Runtime.getRuntime().exec("/home/zeptometer/programs/GomiHiroi/GomiHiroi");
  } catch (IOException e) {
    e.printStackTrace();
  }
  myClient = new Client(this, "127.0.0.1", 5001);
}

void drawref(MemObj from, MemObj to) {
  long fx, fy, tx, ty;
  fx = from.idx/30*10+5;
  fy = from.idx%30*10+300*from.type+5;
  tx = to.idx/30*10+5;
  ty = to.idx%30*10+300*to.type+5;

  strokeWeight(2);

  stroke(255, 50);
  line(fx, fy, tx, ty);
  
}

void draw() {
  receive();

  background(0);
  noStroke();

  for (int i=0; i<env.size(); i++) {
    int x = i/30*10;
    int y = i%30*10;
    MemObj mo = env.get(i);

    assert i == mo.idx;

    if (mo.mark)
      fill(52);
    else
      fill(102);
      
    rect(x, y, 10, 10);
  }

  for (int i=0; i<closure.size(); i++) {
    int x = i/30*10;
    int y = i%30*10+300;
    MemObj mo = closure.get(i);

    assert i == mo.idx;

    if (mo.mark)
      fill(52);
    else
      fill(102);
      
    rect(x, y, 10, 10);
  }

  for (int i=0; i<cons.size(); i++) {
    int x = i/30*10;
    int y = i%30*10+600;
    MemObj mo = cons.get(i);
    
    assert i == mo.idx;
    
    if (mo.mark)
      fill(52);
    else
      fill(102);
      
    rect(x, y, 10, 10);
  }

  for (MemObj mo : table.values())
    for (MemObj to : mo.ref)
      drawref(mo, to);
}
