# numc

Bibliothèque de calcul scientifique en C, conçue pour être construite et
enrichie au fil de l'apprentissage : algèbre linéaire et bilinéaire,
mesure et integration, probabilites, jusqu'a la mécanique quantique.

## Pre-requis

- `gcc` (C11)
- `cmocka` pour les tests unitaires (Fedora/RHEL : `sudo dnf install cmocka-devel`)

## Compilation et tests

| Commande    | Effet                                    |
|-------------|------------------------------------------|
| `make all`  | compile la librairie statique `bin/libnumc.a` |
| `make tests`| compile et execute les tests cmocka      |
| `make valgrind` | execute les tests sous valgrind      |
| `make clean`| supprime les artefacts                   |

## Structure

```
src/            code source de la librairie
    numc.h           tete principale (umbrella header)
    numc_version.h   numero de version
    numc_status.h/c  gestion d'erreur (code retour + dernier statut)
    numc_complex.h/c      nombres complexes
    numc_vector.h/c       vecteurs
    numc_bilinear.h/c     matrices (futures formes bilineaires)
    numc_tensorial.h/c    tenseurs
test/           tests unitaires cmocka
    main.c           runner : execute tous les groupes de tests
    test_<module>.c  groupe de tests d'un module
bin/  lib/  log/    artefacts de compilation (ignores par git)
```

## Conventions

Chaque module = une paire `numc_<module>.h` / `numc_<module>.c` dans `src/`
et un groupe de tests dans `test/test_<module>.c`.

- **Nommage** : tous les symboles publics sont prefixes `numc_` (types
  suffixes `_t`, fonctions `numc_<action>`).
- **Erreurs** : chaque fonction retourne `numc_status_t` (`NUMC_OK` en
  succes) et ecrit ses resultats via des parametres de sortie. Le dernier
  statut est interrogeable via `numc_error()` / `numc_last_status()`.
- **Memoire** : pas d'allocation cachee ; l'appelant fournit les tampons
  de sortie (et leur taille quand c'est pertinent). La bibliotheque ne
  libere jamais ce qu'elle n'a pas alloue.
- **Perturbation d'etat** : pas d'etat global mutable hormis le statut
  d'erreur, qui est `_Thread_local` (C11).
- **Standard** : C11 (`-std=gnu11`), compilation avec `-Wall -Wextra
  -pedantic`.

### Ajouter un module

1. Ecrire `src/numc_<module>.h` et `src/numc_<module>.c`.
2. `make` regenere automatiquement l'umbrella header `src/numc.h`
   (fichier genere, ne pas editer) : il inclut tout header ajoute dans
   `src/`. La cible `tests` est definie comme dependante de
   `$(BINDIR)/$(LIBRARY)`, elle meme dependante des objets, donc la
   regeneration se fait avant toute compilation.
3. Creer `test/test_<module>.c` avec un tableau `const struct CMUnitTest
   numc_<module>_tests[]` et son en-tete `test/test_<module>.h`.
4. Enregistrer le groupe dans `test/main.c`.
5. `make tests`

## Feuille de route

- [x] noyau : statuts d'erreur, version, umbrella header
- [x] nombres complexes (`numc_complex.h`)
- [x] vecteurs (`numc_vector.h`) : construction, acces, add/sub, scale,
      produit scalaire, norme
- [x] matrices (`numc_bilinear.h`) : construction, acces, add/sub, scale,
      produit, norme, transpose
- [x] tenseurs (`numc_tensorial.h`) : construction, add/sub, scale,
      contraction, norme, transpose par permutation
- [x] algebre bilineaire : formes bilineaires, formes quadratiques, formes hermitiennes
- [ ] mesure et integration : quadrature numerique (Riemann, trapeze, Simpson, Gauss-Legendre)
- [ ] probabilites et statistiques : lois usuelles, moments, Monte Carlo
- [ ] mecanique quantique : espaces de Hilbert, operateurs, valeurs propres

## Licence

Voir `LICENSE` (MIT).
