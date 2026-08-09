# Changelog

Toutes les modifications notables de numc sont documentees ici.

Le format s'inspire de [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/),
et ce projet respecte le [Semantic Versioning](https://semver.org/lang/fr/).

## [Unreleased]

- Algebre lineaire et bilineaire (vecteurs, matrices, normes, formes bilineaires)
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
