import processing.net.*;
Client myClient;

void setup() {
  size(200, 200);
  myClient = new Client(this, "127.0.0.1", 5001);
}

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
      System.out.printf("ALLOC : %s : %x\n", objtype, addr); 
    } else if (ops[0].equals("REF")) {
      int from = Integer.parseInt(ops[1].substring(2), 16);
      int to   = Integer.parseInt(ops[2].substring(2), 16);
      System.out.printf("REF : %x : %x\n", from, to);
    } else if (ops[0].equals("DEREF")) {
      int from = Integer.parseInt(ops[1].substring(2), 16);
      int to   = Integer.parseInt(ops[2].substring(2), 16);
      System.out.printf("DEREF : %x : %x\n", from, to);
    } else if (ops[0].equals("MARK")) {
      int addr = Integer.parseInt(ops[1].substring(2), 16);
      System.out.printf("MARK : %x\n", addr);
    } else if (ops[0].equals("SWEEP")) {
      println("SWEEP");
    } else {
      print("hoge");
      print(ops[0].equals("SWEEP"));
      println("poyo");
    }
  }
}

void draw() {
  receive();
}
