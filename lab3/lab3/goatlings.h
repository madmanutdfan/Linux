enum status { ALIVE, DEAD };

class Goatling {
public:
	         Goatling    ();
	int      getNumber   ();
	void     setState    (status state);
	status   getState    ();
	         ~Goatling   ();

private:
	status   state;
};

const int    nGoatlings = 10;
int          nDead      = 0;

Goatling*    goatlings[nGoatlings];
status       stateGoatling;