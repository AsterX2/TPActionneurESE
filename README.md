# TPActionneurESE

- # Compte Rendu : Contrôle de MCC avec la Carte Nucleo-G474RE

  ## Sommaire
  - [Introduction](#introduction)
  - [Objectifs](#objectifs)
  - [6.1 Génération de 4 PWM](#61-génération-de-4-pwm)
  - [6.2 Commande de Vitesse via UART](#62-commande-de-vitesse-via-uart)
  - [6.3 Premiers Tests de Contrôle du Moteur](#63-premiers-tests-de-contrôle-du-moteur)
  - [Conclusion](#conclusion)

  ---

  ## Introduction

  Ce document rend compte de la configuration et des tests d'un système de contrôle moteur en boucle ouverte utilisant une carte Nucleo-G474RE.

  ## Objectifs

  1. Générer des PWM pour le contrôle moteur.
  2. Implémenter une interface de commande de vitesse via UART.
  3. Valider le fonctionnement pratique du système.

  ---

  ## 6.1 Génération de 4 PWM

  Cablage de la sonde numérique :
  
  marron : pb13
  
  orange : pb14
  
  noir : pa8
  
  rouge:  pa9
  
  
  
  
  
  
  
  **Paramètres configurés :**
  
  - **Fréquence** : 20 kHz
  - **Résolution** : 10 bits
  - **Temps mort** : 200 ns, validé par la datasheet des transistors (Rise Time + Turn on delay Time=90 ns).

  **Étapes effectuées :**
  - Configuration des pins pour les canaux PWM de TIM1.
  - Paramétrage des sorties complémentaires et des temps morts dans CubeIDE.

  

  f_PWM = 170,000,000 / (2 * 2 * (2124 + 1))

  
  
  f_PWM = 170,000,000 / 8500 = 20,000 Hz
  
  **Résultats :**
  - Signaux PWM observés et validés à l'oscilloscope (voir illustration ci-dessous).
  
  **Illustrations :**
  
  <img src="./WhatsApp%20Image%202024-11-06%20at%2019.20.49-1730977352039-2.jpeg" alt="WhatsApp Image 2024-11-06 at 19.20.49" style="zoom: 25%;" />

  ![image-20241107120818936](./image-20241107120818936.png)

  ---

  <img src="./image-20241107183303442.png" alt="image-20241107183303442" style="zoom: 50%;" />

  ## 6.2 Commande de Vitesse via UART

  **Commande implémentée :**
  - **Format** : `speed XXXX` pour ajuster la vitesse du moteur.
  
  **Étapes effectuées :**
  
  - Configuration de l’UART pour communication série avec terminal.
  - Détection et traitement de la commande `speed` via putty.
  - Application de la vitesse demandée par ajustement du rapport cyclique PWM.
  
  
  
  Après avoir lu le token "XXXX" de speed XXXX, on vérifie la conformité avec l'intervalle de valeurs extremum du cdc plus une marge de sécurité.
  
  On peut ensuite modifier le rapport cyclique de nos 4 canaux PWM via les fonctions HAL_TIM_SET_Compare(). Il suffit de modifier les channels 1 et 2 car leurs complémentaires ont le même rapport cyclique.
  
  
  
  **Résultats :**
  
  - La commande de vitesse fonctionne, avec validation des valeurs limites (0 à 2174).
  
  
  
  **Illustrations :**
  
  - ![Capture d'écran du terminal série avec commandes de vitesse](lien_image)
  
  ---
  
  ## 6.3 Premiers Tests de Contrôle du Moteur
  
  **Conditions de test :**
  - Rapports cycliques : 50 % et 70 %.
  - Augmentation graduelle pour éviter les à coups trop fort
  
  **Observations :**
  - Comme attendu les phases U et V se compensent à 50% du fait du mode center alined.
  - Démarrage progressif du moteur sans à-coups grâce à la rampe de montée.
  - Courants d'appel limités, améliorant la sécurité des transistors.
  
  **Vue de oscilloscope à un rapport cyclique de 40% et 60% :**
  
  
  
  
  
  ![tek00000](./repoimg/tek00000.png)
  
  ![tek00001](./repoimg/tek00001.png)
  
  
  
  
  
  ---
  
  
  
  ## Conclusion
  
  La configuration des PWM et de l'interface UART a permis de contrôler efficacement la vitesse du moteur en boucle ouverte. Les tests confirment le bon fonctionnement du système, avec un démarrage progressif permettant d’éviter les à-coups et les surintensités.
  
  