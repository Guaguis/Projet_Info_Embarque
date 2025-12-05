# Projet_Info_Embarquée
Akash HOSSAIN
Ghaith RAIS

une partie du code de la fonction de hachage est reprise d'un code écrit par Alexey NAZAROV, dans le cadre de TP l'année dernière.

COMPILATION
	make

TELEVERSER SUR CARTE
	   make upload

SCHEMA ELECTRIQUE
       le meme qu'au TP5 partie 5, avec comme broche d'entrée D2, numérotée 2 sur la carte Arduino

REPARTITION DES TACHES
	    Ghaith a codé le consentement utilisateur avec bouton, la communication UART, et la gestion protocole CTAP. Akash a codé le reste.

ETAT DE PROGRESSION

     Nous avons voulu tout implémenter, mais dans l'état actuel les tests ne passent pas.
     Nous sommes en train de corriger les erreurs à l'exécution. Le problème un peu étrange que nous rencontrons actuellement est que dans la fonction de ctap.c qui répond à la requete getassertion, la conditionnelle qui appelle memoire_get bloque, tandis que nous avons testé que le fil d'exécution de mémoire_get arrivait bien avant le retour de la fonction.

AUTRE REMARQUES

      La fonction de génération d'aléa n'est prévue que pour fournir 32 octets maximum avant reseed. Or, micro-ecc requiert parfois plus que cela (il fait plusieurs requêtes pour keygen, dont la somme des tailles d'aléa requises dépasse 32 octets). pour gérer les autres fonctionnalités, nous avons fait en sorte que la génération d'aléa n'échoue jamais. Il peut arriver que malgré cela, micro-ecc échoue, nous vous encourageons à ressayer le script de tests si cela arrive, bien que de toutes façon les tests ne passent pas pour l'instant.
