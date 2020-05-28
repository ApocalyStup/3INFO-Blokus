## Installation

- Installer SFML avec la commande :
```bash
sudo apt-get install libsfml-dev
```

- Effectuer la commande suivante : 
```bash
git clone https://gitlab.insa-rennes.fr/Fabien.Lefoye/blokus-2.0.git
```

- A la racine du projet, effectuer la commande :
```bash
make
```

- Puis faire la commande suivante pour lancer le jeu : 
```bash
bin/main
```

## Comment jouer ?

- Sélectionner la pièce que vous voulez placer avec un clique gauche.
- Si jamais des pièces vous dérangent lors de votre manipulation, vous pouvez les déplacer librement avec un clique droit (sans que ça n'interfère avec le jeu).
- Flèche du haut pour effectuer autant de rotations que nécessaire.
- Flèche du bas pour effectuer un "flip" (effet miroir).
- Placer la pièce sur le Board.

## Paramètre de l'IA

- Ligne 424 de main.cpp -> diminuer la valeur du premier paramètre, pour laisser moins de réflexion à l'IA.