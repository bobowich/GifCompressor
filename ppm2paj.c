#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "image.h"
#include <time.h>

#define DIMENIONE_PALETTA 256

typedef struct {
     int x, y , z;//x e y sono le dimensioni della matrice // z dimensione della paletta
     PPMPixel *paletta;
     unsigned char *data;
} PAJImage;

struct nodo
{
    int count;//contatore 
    unsigned char red,green,blue;
    struct nodo *next;
};

PAJImage * ppm2paj(int,PPMImage *);
double distanzaColori(PPMPixel,PPMPixel);
void writePAJ(const char *, PAJImage *);
int listaColori(PPMImage *);
struct nodo* create_list(PPMPixel);
struct nodo* add_to_list(PPMPixel);
int search_in_list(PPMPixel, struct nodo **);


struct nodo * head= NULL;//lista che contiene i colori dell'immagine
struct nodo * curr = NULL;

int main(int argc, char const *argv[])
{
    clock_t begin, end;
    double tempo;
	//per compilare: gcc -Wall ppm2paj.c image.c -o ppm2paj
	begin = clock();
	PPMImage *image;
    PAJImage *image2;
	int colori_inseriti;

    image = readPPM(argv[1]);//lettura immagine PPM da file 
    printf("Preparazione lista colori in corso....\n");
    colori_inseriti = listaColori(image);//creo la lista di colori 
    printf("Immagine composta da %d colori\nCompressione in corso ...",colori_inseriti);
    image2 = ppm2paj(colori_inseriti,image);

    writePAJ(argv[2],image2);
    end = clock();
    tempo = (double)(end - begin); 
    tempo = tempo /1000000;
    printf("Compressione terminata. Tempo impiegato:%f\n",tempo);
    /*PAJImage * image;
    image = (PAJImage *)malloc(sizeof(PAJImage));
    image->x = 4;    image->y = 4;    image->z = 4;
    image->paletta = (PPMPixel *)malloc(sizeof(PPMPixel)*image->z);
    image->data = (char *)malloc(sizeof(char)*image->x*image->y);
    //creazione della paletta
    image->paletta[0].red = 25;    image->paletta[0].green = 25;    image->paletta[0].blue = 25;
    image->paletta[1].red = 0;    image->paletta[1].green = 0;    image->paletta[1].blue = 100;
    image->paletta[2].red = 125;    image->paletta[2].green = 225;    image->paletta[2].blue = 5;
    image->paletta[3].red = 67;    image->paletta[3].green = 200;    image->paletta[3].blue = 11;
    //creazione matrice
    image->data[0] = 0;    image->data[1] = 1;    image->data[2] = 2;    image->data[3] = 3;
    image->data[4] = 0;    image->data[5] = 1;    image->data[6] = 2;    image->data[7] = 3;
    image->data[8] = 0;    image->data[9] = 1;    image->data[10] = 2;    image->data[11] = 3;
    image->data[12] = 0;    image->data[13] = 1;    image->data[14] = 2; image->data[15] = 3;
    //stampa immagine
    writePAJ("out.paj",image);*/

	return 0;
}
PAJImage * ppm2paj(int numero_colori,PPMImage * image){
    //-----------------VARIABILI-----------------------------------
	PAJImage * risultato;//struttura dati che contiene dati per l'immagine di tipo PAJ
	int * paletta_frequenza;//contiene la frequenza dei colori della paletta
    double * matrice_paletta_distanze;//contiene le distanze tra i vari colori della paletta

    int dimensione_paletta;//indica la dimensione della paletta
    int dimensione_matrice_distanze_paletta;//dimensioni della matrice (vettore) che contiene le distanze dei colori della paletta


    int i,j,z,q,w;//indice generico
    int x,y;
    bool trovato;
    //-------------------------------------------------------------
	//calcolo della dimensione della paletta
	if(DIMENIONE_PALETTA < numero_colori) dimensione_paletta = DIMENIONE_PALETTA;
	else dimensione_paletta = numero_colori;

	risultato = (PAJImage *)malloc(sizeof(PAJImage));//allocazione immagine
	risultato->paletta = (PPMPixel *)malloc(sizeof(PPMPixel)* dimensione_paletta);//allocazione paletta
	risultato->data = (unsigned char *)malloc(sizeof(unsigned char) * image->x * image ->y);//alloco la matrice
    paletta_frequenza = (int *)malloc(sizeof(int)* dimensione_paletta);//allocazione paletta_frequenza

    //calcolo la dimensione della matrice triangolare dei coefficienti di distanza della paletta
    dimensione_matrice_distanze_paletta =0;
    for( i=1;i< dimensione_paletta; i++) dimensione_matrice_distanze_paletta += i;

    matrice_paletta_distanze = (double *)malloc(sizeof(double)* dimensione_matrice_distanze_paletta);//allocazione paletta_frequenza;

    //riempio la paletta con i primi dimensione_paletta colori
    curr = head;//porto il puntatore utilizzabile della lista alla testa
    i=0;
    while(i< dimensione_paletta){
        //inserisco il colore dalla lista al vettore palette
        risultato->paletta[i].red = curr->red;
        risultato->paletta[i].green = curr->green;
        risultato->paletta[i].blue = curr->blue;

        //inserisco la frequenza del colore della lista in paletta_colori_frequenza
        paletta_frequenza[i] = curr->count;

        curr = curr->next;//aggiorno la posizione del puntatore della lista al nuovo colore
        i++;
    }

    //fase di ottimizzazione dei colori -> eseguita se la paletta non è totalità dei colori inseriti
    if(numero_colori > dimensione_paletta){
        //riempio matrice_coefficienti_paletta con i coefficienti di distanza degli attuali colori della paletta stessa
        z=0;//indice di inSerimento dei valori in matrice_coefficienti_paletta
        for(i = 0;i < dimensione_paletta; i++){
            for(j = i+1; j < dimensione_paletta; j++){
                matrice_paletta_distanze[z] = distanzaColori(risultato->paletta[i],risultato->paletta[j]);
                z++;
            }
        }
        
        /*prelevo ogni colore dalla lista e vedo se devo inserirlo all'interno della palette(sostituendo un'altro colore)
        oppure compattarlo con uno già esistente, scambiandolo in caso questo nuovo abbia frequenza maggiore*/
        PPMPixel tmpP; int tmpFrequenza;
        int distanza_migliore_posizione; 
        double * coefficienti_distanza_newColor;
        coefficienti_distanza_newColor = (double *)malloc(sizeof(double)*dimensione_paletta);
        while(curr != NULL){
            //prelevo il colore dalla lista e la sua frequenza
            tmpP.red = curr->red;
            tmpP.green = curr->green;
            tmpP.blue = curr->blue;
            tmpFrequenza = curr->count;

            //calcolo la distanza dai colori della paletta selezionando la posizione di distanza minima
            distanza_migliore_posizione = 0;
            for(i=0; i < dimensione_paletta; i++){
                coefficienti_distanza_newColor[i] = distanzaColori(tmpP,risultato->paletta[i]);

                if(i != 0){
                    if(coefficienti_distanza_newColor[i] < coefficienti_distanza_newColor[distanza_migliore_posizione])
                        distanza_migliore_posizione = i;
                }
            }

            /*//stampo la matrice delle distanze della paletta
            printf("Matrice distanze paletta\n");
            z=0;
            for(i = 0;i < dimensione_paletta; i++){
                for(j = i+1; j < dimensione_paletta; j++){
                    printf("%d;%d:  %f\n",i,j,matrice_paletta_distanze[z]);
                    z++;
                }
            }
            printf("Vettore distanze da paletta\n");
            for(i=0;i<dimensione_paletta;i++)printf("%f\n",coefficienti_distanza_newColor[i]);  */

            //ora calcolo la posizione dell'elemento migliore della matrice_coefficienti_paletta
            //posizione dell'elemento migliore della matrice in  "  z  "
            z = 0;
            for(i=1; i < dimensione_matrice_distanze_paletta; i++){
                if(matrice_paletta_distanze[i] < matrice_paletta_distanze[z]) z = i;
            }

            /*ora conosco la posizione del coefficiente migliore tra i coefficienti della paletta e la posizione del coefficiente
            migliore del colore nuovo rispetto alla paletta stessa*/
            if(matrice_paletta_distanze[z] < coefficienti_distanza_newColor[distanza_migliore_posizione]){
                //compatto i 2 colori che han dato il coefficiente alla posizione z e aggiorno i coefficienti
                
                //calcolo i 2 colori che han dato la distanza minore in z e li inserisco in x e y
                trovato = false;
                i = dimensione_paletta - 1;
                j = i;
                x = 0;
                y = 0;
                while(!trovato){//calcolo di x
                    if(z < j)   trovato = true;
                    else    x++;
                    i--;
                    j += i;
                }
                j=0;q =0;i = dimensione_paletta -1;
                while(q<x){
                    q++;
                    j += i;
                    i--;
                }
                y = z - j + x + 1;           
                /*ora conosco i 2 colori della paletta che sono più vicini. Unisco quello con frequenza minore a quello di frequenza 
                maggiore. Inserisco successivamente il nuovo colore al posto di quello con frequenza minore ed infine aggiorno la 
                matrice dei coefficienti delle distanze tra colori della paletta*/
                
                if(paletta_frequenza[x] >= paletta_frequenza[y]){//sostituisco il colore in posizione y
                    paletta_frequenza[x] += paletta_frequenza[y];//sommo le due frequenze

                    paletta_frequenza[y] = tmpFrequenza;//aggiorno la frequenza con quella del nuovo colore inserito

                    //sostituisco con il colore della lista
                    risultato->paletta[y].red = tmpP.red;
                    risultato->paletta[y].green = tmpP.green;
                    risultato->paletta[y].blue = tmpP.blue;

                    //Bene. Ora aggiorno i coefficienti della matrice con quelli del nuovo colore 
                    j =x - 1;//utilizzata per calcolare la posizione nel vettore
                    z = dimensione_paletta - 2;
                    for(i = 0; i< dimensione_paletta; i++){
                        if(i < x){
                            if(i != 0){
                                j+= z;
                                z--; 
                            }
                            matrice_paletta_distanze[j] = coefficienti_distanza_newColor[i];
                        }
                        else if(i > x){
                            j++;//calcolo la nuova posizione
                            matrice_paletta_distanze[j] = coefficienti_distanza_newColor[i];
                        }
                        else{//i == x
                            //ricalcolo j
                            w = dimensione_paletta - 1;
                            j =0;
                            for(q = 0; q < x; q++){
                                j += w;
                                w--;
                            }
                            j--;
                        }
                    }
                }
                else{
                    paletta_frequenza[y] += paletta_frequenza[x];//sommo le due frequenze
            
                    paletta_frequenza[x] = tmpFrequenza;//aggiorno la frequenza con quella del nuovo colore inserito

                    //sostituisco con il colore della lista
                    risultato->paletta[x].red = tmpP.red;
                    risultato->paletta[x].green = tmpP.green;
                    risultato->paletta[x].blue = tmpP.blue;

                    //Bene. Ora aggiorno i coefficienti della matrice con quelli del nuovo colore 
                    j = y - 1;//utilizzata per calcolare la posizione nel vettore
                    z = dimensione_paletta - 2;
                    for(i = 0; i< dimensione_paletta; i++){
                        if(i < y){
                            if(i != 0){
                                j+= z;
                                z--; 
                            }
                            matrice_paletta_distanze[j] = coefficienti_distanza_newColor[i];
                        }
                        else if(i > y){
                            j++;//calcolo la nuova posizione
                            matrice_paletta_distanze[j] = coefficienti_distanza_newColor[i];
                        }
                        else{//i == y
                            //ricalcolo j
                            w = dimensione_paletta - 1;
                            j =0;
                            for(q = 0; q < y; q++){
                                j += w;
                                w--;
                            }
                            j--;
                        }
                    }
                }
            }
            else{
                if(paletta_frequenza[distanza_migliore_posizione] < tmpFrequenza){

                    //aggiorno il colore con il nuovo
                    risultato->paletta[distanza_migliore_posizione].red = tmpP.red;
                    risultato->paletta[distanza_migliore_posizione].green = tmpP.green;
                    risultato->paletta[distanza_migliore_posizione].blue = tmpP.blue;
                }
                                
                paletta_frequenza[distanza_migliore_posizione] += tmpFrequenza;//aggiorno la frequenza
            }
            curr = curr->next;
        }
    }//terminazione if - fase di ottimizzazione 
    
    //OK!! ora ho la paletta al completo, non mi resta che costruire la matrice di riferimento
    printf("Coversione immagine...\n");
    risultato->x = image->x;
    risultato->y = image->y;
    risultato->z = dimensione_paletta;

    int posizione =0; double tmpDistanza;
    for(i = 0; i < image->x * image->y; i++){
        posizione = 0;
        trovato = false;
        tmpDistanza = distanzaColori(image->data[i],risultato->paletta[0]);
        //printf("%f\n",tmpDistanza);
        for (j = 1; j < risultato->z && !trovato; j++){
            //printf("%f\n",distanzaColori(image->data[i],risultato->paletta[j]) );
            if(tmpDistanza > distanzaColori(image->data[i],risultato->paletta[j])){
                tmpDistanza = distanzaColori(image->data[i],risultato->paletta[j]);
                posizione = j;
                if(tmpDistanza == 0) trovato = true;
            }
        }
        risultato->data[i]= posizione;  
        //printf("%d    ",posizione );  
    }

    /*printf("Stampa paletta\n");
    for(i=0; i < risultato->z ; i++)
        printf("%d:    %d %d %d   |",i,risultato->paletta[i].red,risultato->paletta[i].green,risultato->paletta[i].blue);*/
        //printf("%d %d %d\n",risultato->paletta[214].red,risultato->paletta[214].green,risultato->paletta[214].blue);
        //for(i = 0; i < image->x * image->y; i++) printf("%d\n",risultato->data[i]);

	return risultato;
}

double distanzaColori(PPMPixel p1,PPMPixel p2){
    double ris;
    int c1 = p1.red - p2.red;
    int c2 = p1.green -p2.green;
    int c3 = p1.blue -p2.blue;
    ris = (c1*c1 + c2*c2 + c3*c3); 
    ris = sqrt(ris);
    return  ris;
/*
    //conversione RGB a YUV
    double Wr = 0.299;
    double Wb = 0.114;
    double Wg = 0.587;

    double Yp1 = p1.red*Wr + p1.green*Wg + p1.blue*Wb;
    double Yp2 = p2.red*Wr + p2.green*Wg + p2.blue*Wb;

    return abs(Yp1 - Yp2);
*/
}

void writePAJ(const char * filename, PAJImage * image){
	FILE * fp;

	//open file for output
    fp = fopen(filename, "wb");
    if (!fp) {
         fprintf(stderr, "Unable to open file '%s'\n", filename);
         exit(1);
    }

    //write the header file
    //image format
    fprintf(fp, "PAJ\n");

    //comments
    fprintf(fp, "# Created by PAJOLA\n");

    //image size
    fprintf(fp, "%d %d %d\n",image->x,image->y, image->z);

    //stampa della paletta
    fwrite(image->paletta,3,image->z,fp);

    //stampa matrice di riferimento
    fwrite(image->data,1,image->x * image->y,fp);

    fclose(fp);
}

int listaColori(PPMImage * image){
	int numero_colori =0;
	PPMPixel pix;
	int i=0;
    for(i =0; i < image->x * image->y;i++){
        pix.red = image->data[i].red;
        pix.green = image->data[i].green;
        pix.blue = image->data[i].blue;
        //printf("%d %d %d\n",pix.red, pix.green, pix.blue );
        if(search_in_list(pix, NULL) == 0)//elemento non trovato
            {
                add_to_list(pix);
                numero_colori++;
            }
    }
    return numero_colori;
}

struct nodo* create_list(PPMPixel pix){
    struct nodo *ptr = (struct nodo*)malloc(sizeof(struct nodo));
    ptr->red = pix.red;
    ptr->green = pix.green;
    ptr->blue = pix.blue;
    ptr->count =1;
    ptr->next = NULL;

    head = curr = ptr;
    return ptr;
}

struct nodo* add_to_list(PPMPixel pix){
    if(NULL == head)
    {
        return (create_list(pix));
    }
    struct nodo *ptr = (struct nodo*)malloc(sizeof(struct nodo));
    ptr->red = pix.red;
    ptr->green = pix.green;
    ptr->blue = pix.blue;
    ptr->count = 1;
    ptr->next = NULL;
    
    curr->next = ptr;
    curr = ptr;
    
    return ptr;
}

int search_in_list(PPMPixel pix, struct nodo **prev){
    struct nodo *ptr = head;
    struct nodo *tmp = NULL;
    bool found = false;

    while(ptr != NULL)
    {
        if(ptr->red == pix.red && ptr->green == pix.green && ptr->blue == pix.blue)
        {
            //printf("trovato: %d %d %d\n",ptr->red,ptr->green,ptr->blue);
            //printf("%d\n",ptr->count);
            ptr->count++;
            found = true;
            break;
        }
        else
        {
            tmp = ptr;
            ptr = ptr->next;
        }
    }

    if(true == found)
    {
        if(prev)
            *prev = tmp;
        return 1;
    }
    else
    {
        return 0;
    }
}