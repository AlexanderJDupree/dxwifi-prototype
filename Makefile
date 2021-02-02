# OreSat Live Build Targets

tx: dxwifi/tx.c dxwifi/cli.c
	gcc -o bin/tx dxwifi/tx.c dxwifi/cli.c -I .