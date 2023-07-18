color cc = #000000;
color sc = #000000;
color gc = #000010;

int pTime;
int startingTime, finalTime;
boolean loop = true;

void setup(){
  size(400,400);
  frameRate(200);
  startingTime = millis();
  pTime = startingTime;
  finalTime = pTime + 5000;
}

void draw(){
  if(loop && (millis()-pTime >= 20)){
    print("-");
    pTime = millis();
    float timeLeft = finalTime - millis();
    if(timeLeft <= 20){
      cc = gc;
      loop = false;
    }else{
      int dur = finalTime - startingTime;
      float precentThrough = (pTime - startingTime) / (float)dur;
      int r = (int)(precentThrough * (red(gc) - red(sc)));
      int g = (int)(precentThrough * (green(gc) - green(sc)));
      int b = (int)(precentThrough * (blue(gc) - blue(sc)));
      cc = color(red(sc) + r, green(sc) + g, blue(sc) + b);
    }
  }
  background(gc);
  fill(cc);
  rect(width/2,0,width/2,height);
}

void keyPressed(){
  if(key == 's'){
    startingTime = millis();
    finalTime = startingTime + 1000;
    println("final time: " + finalTime);
    gc = color(random(0,255),random(0,255),random(0,255));
    sc = cc;
    //dc = color(red(gc)-red(cc),reen(gc)-green(cc),blue(gc)-blue(cc));
    loop = true;
  }
}