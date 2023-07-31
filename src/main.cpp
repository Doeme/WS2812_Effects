#include <NeoPixelBus.h>

static constexpr int N=300*3;
NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> strip(N,  2); //, 17, 16);

float fract(float x) { return x - int(x); }

float mix(float a, float b, float t) { return a + (b - a) * t; }

float step(float e, float x) { return x < e ? 0.0 : 1.0; }

RgbColor hsv2rgb(float h, float s, float b) {
  float r = b * mix(1.0, constrain(abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  float g = b * mix(1.0, constrain(abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  float bb = b * mix(1.0, constrain(abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  return RgbColor(255*r, 255*g, 255*bb);
}

class Effect{
public:
	virtual bool frame() = 0;
	virtual void reset() = 0;
};

class Gay : public Effect{
	float phase;
	int count;
	void reset(){
		phase = 0;
		count = 10;
	}
	
	bool frame(){
		phase += 0.01;
		if(phase > 1.0){
			phase = 0.0;
			if(--count==0){
				return true;
			}
		}
		
		for(int i=0; i<N; i++){
			float p = ((float)i)/N*10;
			strip.SetPixelColor(i, hsv2rgb(p+phase,1.0,1.0));
		}
		
		return false;
	}
};

class Freeze : public Effect{
	float state[N];
	int count;
	void reset(){
		for(int i=0; i<N; i++){
				state[i] = 0;
		}
		for(int i=0; i<7; i++){
			state[random(N-1)] = 1.0;
		}
	}
	
	bool frame(){
		static constexpr float alpha=0.05;
		bool upd = true;
		for(int i=0; i<N; i++){
			float v = state[i];
			strip.SetPixelColor(i, RgbColor(255*v, 255*v, 255*v));
			if(i>0){
				state[i] += state[i-1]*alpha;
			}
			if(i<N-1){
				state[i] += state[i+1]*alpha;
			}
			if(state[i] >= 1.0){
				state[i] = 1.0;
			}
			else{
				upd = false;
			}
		}
		
		return upd;
	}
};

class Ball: public Effect{
	int pos;
	int dt;
	int dt0=2;
	int count=10;
	public:
	Ball(int dt0): dt0(dt0){};

	void reset(){
		pos = 0;
		dt = dt0;
		count = 3;
	}
	
	bool frame(){
		for(int i = 0; i<N; i++){
			float h,s,v;
			if( abs(i-pos) < 20 ) {
				float frommid = abs(i-N/2);
				float rel = frommid/(N/2.0);
				rel = 1-rel;
				
				v = expf(-((i-pos)*(i-pos))/(1+5*rel));
				if(i<pos){
					h = 0.1;
				}else{
					h = 0.7;
				}
				s = 1-v;

			}
			else {
				v = 0;
				h = 0;
				s = 0;
			}
			strip.SetPixelColor(i, hsv2rgb(h,s,v));
		}
		pos += dt;
		if(pos >= N || pos < 0){
			dt *= -1;
			pos += dt;
			if( --count < 0 ) {
				return true;
			}
		}
		return false;
	}
};

class Ball: public Effect{
	int pos;
	int dt;
	int dt0=2;
	int count=10;
	public:
	Ball(int dt0): dt0(dt0){};

	void reset(){
		pos = 0;
		dt = dt0;
		count = 3;
	}
	
	bool frame(){
		for(int i = 0; i<N; i++){
			float h,s,v;
			if( abs(i-pos) < 20 ) {
				float frommid = abs(i-N/2);
				float rel = frommid/(N/2.0);
				rel = 1-rel;
				
				v = expf(-((i-pos)*(i-pos))/(1+5*rel));
				if(i<pos){
					h = 0.1;
				}else{
					h = 0.7;
				}
				s = 1-v;

			}
			else {
				v = 0;
				h = 0;
				s = 0;
			}
			strip.SetPixelColor(i, hsv2rgb(h,s,v));
		}
		pos += dt;
		if(pos >= N || pos < 0){
			dt *= -1;
			pos += dt;
			if( --count < 0 ) {
				return true;
			}
		}
		return false;
	}
};

#define NUM_EFFECTS 4

Gay gay;
Ball ball(3);
Balls balls(3);
Freeze freeze;

Effect *effects[NUM_EFFECTS]={&balls, &ball, &freeze, &gay};

void setup(){
	Serial.begin(115200);
	strip.Begin();
	strip.ClearTo(RgbColor(0,0,0));
	strip.Show();
	effects[0]->reset();
}

void loop(){
	static unsigned long last_micros=0;
	static unsigned int i=0;
	Effect *curEffect = effects[i];
	if(micros() > last_micros + 1000000/120){
		if(curEffect->frame()){
			i += 1;
			if(i >= NUM_EFFECTS){
				i = 0;
			}
			effects[i]->reset();
		}
		strip.Show();
		//Serial.print(micros()-last_micros);
		//Serial.print("\r\n");
		last_micros = micros();
	}
}
