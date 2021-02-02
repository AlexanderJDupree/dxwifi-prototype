# OreSat Live Build Targets

tx: dxwifi/tx.c dxwifi/cli.c dxwifi/dxwifi.c
	gcc -o bin/tx dxwifi/dxwifi.c dxwifi/tx.c dxwifi/cli.c -lpcap -I .