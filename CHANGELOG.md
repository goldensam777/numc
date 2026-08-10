# Changelog

Toutes les modifications notables de numc sont documentees ici.

Le format s'inspire de [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/),
et ce projet respecte le [Semantic Versioning](https://semver.org/lang/fr/).

## [Unreleased]

### Ajoute

- **Algebre multilineaire, decoupee en trois modules** :
  - `numc_vector.h` : `numc_vector_t` (construction, acces verifie
    `get`/`set`, `add`, `sub`, `scale`, `dot`, norme euclidienne) ;
  - `numc_bilinear.h` : `numc_matrix_t` (construction, acces verifie
    `get`/`set`, `add`, `sub`, `scale`, `mul`, norme de Frobenius,
    `transpose`) ;
  - `numc_tensorial.h` : `numc_tensor_t` (construction avec rejet des
    dimensions nulles, `add`, `sub`, `scale`, `contract`, norme de
    Frobenius, `transpose` par permutation).
  - Les trois types partagent la meme convention d'ownership (les
    `numc_*_from` copient les donnees).
- **Nouveaux codes d'erreur** : `NUMC_ERR_DIM_MISMATCH` et
  `NUMC_ERR_OUT_OF_BOUNDS`, messages associes dans `numc_status_string()`.
- **Fiabilisation** : chaque objet porte un tag `magic` (pose par
  `numc_*_from`, efface par `numc_*_destroy`) ; `numc_*_is_valid()`
  detecte les objets non initialises ou detruits ; messages d'erreur via
  `numc_set_error()`.
- **Integration continue** : workflow GitHub Actions (gcc et clang) qui
  compile, lance les tests et le controle de fuites valgrind
  (`--error-exitcode=1`).
- **Tests** : 66 tests unitaires (5 noyau, 41 complexes, 8 vecteurs,
  5 matrices, 7 tenseurs).

### A venir

- Algebre bilineaire : formes bilineaires, formes quadratiques, formes hermitiennes
- Mesure et integration numerique (quadrature)
- Probabilites et statistiques (lois, moments, Monte Carlo)
- Mecanique quantique (espaces de Hilbert, operateurs)

## [0.1.0] - 2026-08-09

### Ajoute

- Conversion du template CLI en **bibliotheque statique pure** (`bin/libnumc.a`).
- **Umbrella header** `src/numc.h`, genere automatiquement par le Makefile a
  partir de toutes les tetes publiques de `src/` (regeneration forcee, robuste
  aux ajouts et retraits de headers).
- **Gestion d'erreur** : `numc_status_t` (codes retour), dernier statut
  thread-local interrogeable via `numc_error()` / `numc_last_status()`,
  message lisible via `numc_status_string()`.
- **Infrastructure de tests** : runner cmocka, chaque module expose
  `numc_<module>_tests_run()` ; cibles `make tests` et `make valgrind`.
- **Module nombres complexes** (`numc_complex_t`) :
  - constructeurs : `from`, `from_polar`, `zero`, `one`, `i` ;
  - arithmetique : `add`, `sub`, `mul`, `conj`, `div` (algorithme de Smith,
    robuste aux debordements), `pow` (exposant entier par elevation au carre) ;
  - grandeur reelle : `abs` (via `hypot`, sans debordement), `arg` ;
  - comparaisons : `is_zero`, `is_finite`, `equal` (tolerance absolue) ;
  - transcendantes : `sqrt` (branche principale, formule stable sans trig),
    `exp`, `log` (logarithme principal) ;
  - puissance complexe : `cpow` ;
  - trigonometrie : `sin`, `cos`, `tan` (poles reconnus) ;
  - hyperboliques : `sinh`, `cosh`.

### Notes de conception

- Standard C11 (`-std=gnu11`), compilation avec `-Wall -Wextra -pedantic`.
- Convention : les fonctions retournent `numc_status_t` et ecrivent leurs
  resultats dans des parametres de sortie ; pas d'allocation cachee.
- 36 tests unitaires (5 pour le noyau, 31 pour les complexes).
