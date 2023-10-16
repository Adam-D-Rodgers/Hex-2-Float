#include "hex-2-float.h"


int main (int argc, char * argv[]) {
	FILE *ifile = stdin;
	char input[200] = {0};
	char * val = NULL;

	long exp_bits = F_EXP_BITS_DEFAULT;
	long frac_bits = F_FRAC_BITS_DEFAULT;
	double exp_bias = 0.0;
	double frac_add = F_FRAC_ADD_DEFAULT;

	{
		int opt = 0;

		while ((opt = getopt(argc, argv, GETOPT_STRING)) != -1) {
			switch (opt) {
				case 'i':
					ifile = fopen(optarg, "r");
					if(!ifile) {
						fprintf(stderr, "FAILED TO OPEN FILE");
						return EXIT_FAILURE;
					}
					break;
				case 'd':
					exp_bits = 11;
					frac_bits = 52;
					break;
				case 'h':
					exp_bits = 5;
					frac_bits = 10;
					break;
				case 'b':
					exp_bits = 8;
					frac_bits = 7;
					break;
				case 'm':
					exp_bits = 4;
					frac_bits = 3;
					exp_bias = -2;
					break;
				case 'e':
					exp_bits = strtod(optarg, NULL);
					break;
				case 'E':
					exp_bias = strtod(optarg, NULL);	
					break;
				case 'f':
					frac_bits = strtod(optarg, NULL);
					break;
				case 'F':
					frac_add = strtod(optarg, NULL);
					break;
				case 'v':
					fprintf(stderr, "using verbose...it doesnt do much");
					break;	
				case 'H':
					printf("Usage: ./hex-2-float [OPTION ...]\n");
					printf("\t\tSettings default to single precision (float, 32-bits, 1-8-23)\n");
					printf("\t\tInput defaults to stdin if -i <file_name> is not used\n");
					printf("\t\tUnless otherwise specified, the bias follows IEEE 754 rules\n");
					printf("\t-i <file_name> specify the name of the input file\n");
					printf("\t-d   use settings for double precision (double, 64-bits, 1-11-52)\n");
					printf("\t-h   use settings for half precision (_Float16, 16-bits, 1-5-10)\n");
					printf("\t-b   use settings for half precision (bfloat16, 16-bits, 1-8-7)\n");
					printf("\t-m   use settings for quarter precision (minifloat, 8-bits, 1-4-3  bias -2)\n");
					printf("\t-e # set the number of bits to use for the exponent\n");
					printf("\t-E # set the value used for the exponent bias\n");
					printf("\t-f # set the number of bits to use for the fraction\n");
					printf("\t-F # set the value to add to the fraction (unstored fraction bits, normalized form)\n");
					printf("\t-B   read the input as binary. Spaces are okay between fields, but no trailing comments (eg 1 001 1101)\n");
					printf("\t-v   print the settings (to stderr) before reading input\n");
					printf("\t-H   display this most wonderful message and exit\n");
					return EXIT_SUCCESS;
				default: 
					break;
			}
		}
	}
	//getting the variable
	val = fgets(input, 200, ifile);
	input[strlen(input) - 1] = '\0';
	while(val != NULL){
		unsigned long sign_mask = 0;
		unsigned long hex = 0;
		unsigned long exp_mask = 0;
		unsigned long exp = 0;
		unsigned long frac_mask = 0; 
		double M = 0.0;
		double E = 0;
		double frac = 0.0;
		double final_val = 0.0;
		double unbias_exp = 0;
		int form = 1;
		int sign = 0;



		sscanf(input, "%lx", &hex);

		//making the masks	
		sign_mask = 0x1lu << (frac_bits + exp_bits);
		exp_mask = 0x1lu;
		for(int i = 0; i < exp_bits - 1; i++){
			exp_mask <<= 1;
			exp_mask |= 0x1lu;
		}
		exp_mask <<= frac_bits;
		frac_mask = (1ull << frac_bits) - 1;

		//setting all the veriables
		exp = hex & exp_mask;

		if(exp == 0){
			form = 0;
		}
		if(exp == exp_mask){
			form = 2;
		}
		unbias_exp = exp >> frac_bits;	
		if(hex & sign_mask){
			sign = 1;
		}
		else {
			sign = 0;
		}

		if(exp_bias == 0.0) exp_bias = (pow(2.0, exp_bits - 1)) - 1;


		//Normalized, Denormalized, and Special Values sorting
		if(form == 1){ //normalized
			E = unbias_exp - exp_bias;	
			frac = ((double)(hex & frac_mask))/(1ull << frac_bits);	
			M = frac_add + frac;		
			final_val = M * pow(2.0, E);
			if(sign == 1) final_val *= -1;
		}
		else if (form == 0) { //denormalized
			E = 1 - exp_bias;
			frac = ((double)(hex & frac_mask))/(1ull << frac_bits);	
			M = frac;
			final_val = M * pow(2.0, E);
			if(sign == 1) final_val *= -1;
		}
		else if (form == 2) { //special value
			frac = ((double)(hex & frac_mask))/(1ull << frac_bits);	
			final_val = 0;
			final_val = 0.0;
		}




		//trying to print it by bits
		printf("%s\n", input);
		printf("\t%d ", (hex & sign_mask) ? 1 : 0);
		for(int i = exp_bits; i > 0; i--) {
			sign_mask >>= 1;
			printf("%d", (hex & sign_mask) ? 1 : 0);
		}	
		printf(" ");
		for (int i = frac_bits; i > 0; i--) {
			sign_mask >>= 1;
			printf("%d", (hex & sign_mask) ? 1 : 0);
		}
		//printing diffrent variables and format
		printf("\n\ts ");
		for (int i = 0; i < exp_bits; i++){
			printf("e");
		}
		printf(" ");
		for (int i = 0; i < frac_bits; i++){
			printf("f");
		}
		printf("\n");
		if (form == 1){
			printf("\tnormalized value\n");
		}
		else if(form == 0){
			printf("\tdenormalized value\n");
		}
		else if(form == 2){
			printf("\tspecial value\n");
			if(frac == 0){	
				if(sign == 0){
					printf("\tpositive infinity\n\n");
				}
				else {
					printf("\tnegative infinity\n\n");
				}
			}
			else { 
				printf("\tNaN\n\n");
			}

		}
		if(form != 2) {
			if(sign == 0){
				printf("\tsign:\t\t%s\n", "positive");
			}
			else {
				printf("\tsign:\t\t%s\n", "negative");
			}

			printf("\tbias:\t\t%-10.0lf\n", exp_bias);
			printf("\tunbiased exp:\t%-10.0lf\n", unbias_exp);
			printf("\tE:\t\t%-10.0lf\n", E);
			printf("\tfrac:\t\t%-.20lf\n", frac);
			printf("\tM:\t\t%-.20lf\n", M);
			printf("\tvalue:\t\t%-.20lf\n", final_val);
			printf("\tvalue:\t\t%-.20le\n\n", final_val);
		}
		val = fgets(input, 200, ifile);
		input[strlen(input) - 1] = '\0';

	}

	return EXIT_SUCCESS;
}

