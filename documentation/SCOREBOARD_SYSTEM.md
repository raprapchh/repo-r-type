# Système de Scoreboard R-Type

## Vue d'ensemble

Un système complet de scoreboard a été implémenté pour sauvegarder et afficher les scores des joueurs après chaque partie.

## Architecture

### 1. Structures de données (`shared/ScoreData.hpp` et `.cpp`)

**ScoreEntry** - Représente une entrée de score unique:
- `player_name`: Nom du joueur
- `score`: Score obtenu
- `timestamp`: Horodatage Unix de la partie

**ScoreboardData** - Conteneur pour tous les scores:
- `solo_scores`: Vector des meilleurs scores en mode solo (top 10)
- `multi_scores`: Vector des meilleurs scores en mode multijoueur (top 10)
- Méthodes pour ajouter et trier automatiquement les scores

### 2. Gestionnaire de scoreboard (`client/include/ScoreboardManager.hpp` et `.cpp`)

**Fonctionnalités**:
- **Chargement**: Lit les scores depuis `client/saves/scoreboard.txt`
- **Sauvegarde**: Écrit les scores dans le fichier texte
- **Ajout de scores**: Méthodes séparées pour solo et multi
- **Format de fichier**: Format texte simple et lisible

**Format du fichier de sauvegarde**:
```
# R-Type Scoreboard Save File
# Format: PlayerName:Score:Timestamp

[SOLO]
PlayerName:12345:1234567890

[MULTI]
PlayerName:54321:1234567890
```

### 3. État de scoreboard (`client/include/ScoreboardState.hpp` et `.cpp`)

**Interface utilisateur**:
- Titre principal "SCOREBOARD"
- Deux onglets: SOLO et MULTI
- Affichage classé des scores (rang, nom, score)
- Bouton "BACK" pour revenir au menu
- Navigation par clic ou touche Tab

**Fonctionnalités**:
- Affiche jusqu'à 10 meilleurs scores par catégorie
- Interface réactive avec redimensionnement
- Navigation intuitive entre les onglets

### 4. Intégration avec le jeu

**Dans Client.hpp/cpp**:
- Ajout d'un membre `ScoreboardManager scoreboard_manager_`
- Chargement automatique des scores au démarrage
- Méthode `get_scoreboard_manager()` pour accès global

**Dans MenuState.hpp/cpp**:
- Nouveau bouton "SCOREBOARD"
- Navigation vers ScoreboardState via clic
- Bouton positionné entre QUIT et le bas de l'écran

**Dans GameState.hpp/cpp**:
- Détection de la mort du joueur
- Récupération du score via le composant `Score`
- Détermination automatique du mode (solo vs multi)
- Sauvegarde unique du score (flag `score_saved_`)
- Réinitialisation du flag lors du retour au menu

**Dans States.hpp**:
- Ajout de `StateType::Scoreboard` dans l'énumération

## Logique de sauvegarde des scores

### Quand un score est sauvegardé
1. Le joueur meurt (`game_over_ = true`)
2. Le score est récupéré du composant `Score` de l'entité joueur
3. Le nombre total de joueurs est compté
4. Le score est ajouté à la catégorie appropriée:
   - **Solo**: Si 1 joueur ou moins
   - **Multi**: Si 2 joueurs ou plus
5. Le score est automatiquement sauvegardé sur disque

### Prévention des doublons
- Flag `score_saved_` garantit une seule sauvegarde par partie
- Réinitialisé lors de l'entrée dans GameState
- Réinitialisé lors du retour au menu

## Utilisation

### Pour le joueur
1. Lancer le jeu
2. Jouer une partie
3. À la mort du joueur, le score est automatiquement sauvegardé
4. Cliquer sur "SCOREBOARD" dans le menu principal
5. Naviguer entre les onglets SOLO et MULTI
6. Cliquer sur "BACK" ou appuyer sur ESC pour revenir au menu

### Pour le développeur
```cpp
// Accéder au scoreboard depuis le client
ScoreboardManager& manager = client.get_scoreboard_manager();

// Ajouter un score manuellement (pour tests)
manager.add_solo_score("TestPlayer", 10000);
manager.add_multi_score("TestPlayer", 20000);

// Charger/Sauvegarder manuellement
manager.load();
manager.save();
```

## Fichiers modifiés

### Nouveaux fichiers
- `shared/ScoreData.hpp`
- `shared/ScoreData.cpp`
- `client/include/ScoreboardManager.hpp`
- `client/src/ScoreboardManager.cpp`
- `client/include/ScoreboardState.hpp`
- `client/src/ScoreboardState.cpp`
- `client/saves/scoreboard.txt` (généré automatiquement)

### Fichiers modifiés
- `client/include/Client.hpp` - Ajout du ScoreboardManager
- `client/src/Client.cpp` - Initialisation et chargement
- `client/include/GameState.hpp` - Flag score_saved_
- `client/src/GameState.cpp` - Logique de sauvegarde
- `client/include/MenuState.hpp` - Bouton scoreboard
- `client/src/MenuState.cpp` - Navigation vers scoreboard
- `client/include/States.hpp` - Enum StateType
- `CMakeLists.txt` - Compilation de shared/*.cpp

## Remarques importantes

### Conformité avec l'architecture
- ✅ Respecte le principe "le render n'est pas implémenté dans le moteur"
- ✅ Utilise le système d'états (IState) existant
- ✅ Utilise le Renderer abstrait pour le dessin
- ✅ Utilise les composants ECS existants (Score)

### Extensibilité future
- Format de fichier facilement extensible
- Possibilité d'ajouter d'autres statistiques (temps de jeu, ennemis tués, etc.)
- Possibilité d'ajouter un système de classement en ligne
- Possibilité de filtrer/trier les scores par date

### Performance
- Chargement asynchrone possible (actuellement synchrone)
- Nombre de scores limité à 10 par catégorie (configurable via MAX_SCORES)
- Tri automatique lors de l'ajout (O(n log n))
- Fichier texte léger et rapide à parser

## Tests recommandés

1. **Test de sauvegarde**: Jouer une partie, mourir, vérifier le fichier
2. **Test solo/multi**: Vérifier que les scores vont dans la bonne catégorie
3. **Test d'affichage**: Vérifier l'interface du scoreboard
4. **Test de navigation**: Vérifier les onglets et le bouton retour
5. **Test de tri**: Ajouter plusieurs scores et vérifier l'ordre
6. **Test de limite**: Ajouter plus de 10 scores, vérifier la troncature
