
char req[41];

// repond a la requete
void respond(void);

// lit la requete -> declenche par interruption a priori, ou bien est bloquant jusqua reception requete
void listen(void);

void makecredential(void);
void getassertion(void); 
void listcredentials(void);
void reset(void); 
