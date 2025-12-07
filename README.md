# Projet_Info_Embarquée
Akash HOSSAIN
Ghaith RAIS

ce projet contient une implémentation de la fonction de hachage BLAKE2S qui est reprise d'un code écrit par Alexey NAZAROV, dans le cadre de TP l'année dernière.

ce projet contient une implémentation d'un système de buffer circulaire écrite par Vincent RUELLO.

ce projet utilise la bibliothèque micro-ecc de Keneth MacKay
   https://github.com/kmackay/micro-ecc

# COMPILATION
	make

# TELEVERSER SUR CARTE
	   make upload

# SCHEMA ELECTRIQUE
       le meme qu'au TP5 partie 5, avec comme broche d'entrée D2, numérotée 2 sur la carte Arduino

# REPARTITION DES TACHES
	    Ghaith a codé le consentement utilisateur avec bouton, la communication UART, et la gestion du protocole CTAP. Akash a géré le reste, c'est à dire la génération d'aléa, l'interaction avec micro-ecc, ainsi que la gestion en mémoire des credentials.

# ETAT DE PROGRESSION
     Le projet est pleinement fonctionnel. Nous avons davantage appauvri notre générateur aléatoire pour qu'il n'échoue pas, sans pour autant renvoyer tout le temps la même chose (plus de détails plus loin).

# DIFFICULTES RENCONTREES
	    Une fonction (memoire_get) écrivait dans l'adresse d'un pointeur donné en argument, au lieu de l'adresse pointée par ce pointeur. En conséquence, on écrivait dans la pile, ce qui faisait crasher le programme au moment du retour. Avant de nous rendre compte de ce problème, nous avons décidé de le contourner en implémentant cette fonction differemment (en utilisant un iterateur), réussissant ainsi à faire fonctionner le projet. Ce n'est que peu de temps avant le rendu final que nous avons trouvé la source du problème et que nous l'avons corrigée. Par défaut, le projet utilise la première solution (avec un itérateur). On peut la remplacer par l'autre solution (correction de memoire_get) en décommentant la première ligne de main.c, les deux implémentations fonctionnent.

# GENERATION ALEA
	   Nous avons trois sources d'aléatoire : la température, les dates où l'utilisateur appuie sur le bouton, et celles où on reçoit une requête du client. Nous avons pensé qu'avoir à la fois des sources d'aléa sensibles aux actions de l'utilisateur et du client rendait le projet plus robuste contre le side-channel. On pourrait aussi partir du principe qu'une personne mal intentionnée en possession de l'Authenticator peut de toute façon s'authentifier où elle veut, et donc qu'il n'est pas déraisonnable de faire comme hypothèse de sécurité que l'attaquant n'est pas en possession de l'Authenticator.
	   Les interactions avec le client et l'utilisateur alimentent le programme en aléa, on aura donc un aléa de meilleur qualité si on a beaucoup de telles interactions sans solliciter le générateur. Au minimum, on exige qu'au moins une interaction avec le client et une avec l'utilisateur soit effectuée pour qu'on puisse reseed (ce qui fait 6 octets d'aléa en comptant la mesure de température).
	   Les trois sources d'aléa sont "uniformisées" avec la fonction de hachage BLAKE2S, la taille du haché est donc de 32 octets (même si l'entropie est inférieure). On utilise comme sel un compteur, qui est celui utilisé pour les cred_id.
	   Nous nous sommes rendu compte que, même si micro-ecc ne demandait jamais d'un coup plus de 32 octets d'aléa, la bibliothèque, en un seul appel de keygen ou sign, faisait plusieurs requêtes au générateur d'aléa, pour une somme totale qui pouvait dépasser 32 octets. Il faudrait donc idéalement utiliser la sortie de BLAKE2S comme graine pour un vrai générateur pseudo-aléa. Pour ne pas compliquer davantage le projet, lorsque les 32 octets d'aléa sont épuisés et qu'il n'y a pas eu d'interaction avec l'utilisateur ou le client, nous avons fait en sorte que le programme fournisse quand même des octets, en incrémentant le sel et en re-hachant. Ceci n'est bien-sûr pas du tout idéal, je ne pense pas qu'une fonction de hachage + un compteur soit considérée comme un bon générateur pseudo-aléa.
