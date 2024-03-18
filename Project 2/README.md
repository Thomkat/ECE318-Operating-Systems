|----------------------------------------------------|
|   Dionisis Kritsas 3210 dkritsas@uth.gr            |
|   Thomas Katraouras 3215 tkatraouras@uth.gr        |
|   Giorgos Margaritis 3356 geormargaritis@uth.gr    |
|----------------------------------------------------|



-------Οδηγίες για SJF με Goodness-------
1. Βάλτε σε comment την γραμμή 127 και βγάλτε απο comment τη γραμμή 124 στο schedule.c .
2. make

-------Οδηγίες για SJF με Min Exp Burst-------
1. Βάλτε σε comment την γραμμή 124 και βγάλτε απο comment τη γραμμή 127 στο schedule.c .
2. make


-------Διαδικασία που ακολουθήσαμε για να λάβουμε μετρήσεις για την μελέτη απόδοσης-------

Στο task_struct, προσθέσαμε 3 βοηθητικά πεδία, τα οποία είναι αποκλειστικά για τις μετρήσεις απόδοσης.
Αυτά είναι:
totalExecutionTime   -->   Ο συνολικός χρόνος εκτέλεσης (το άθροισμα του CPU time της διεργασίας)
totalProcessingTime  -->   Ο συνολικός χρόνος διεκπεραίωσης (η διαφορά του χρόνου που "σκοτώθηκε" η εργασία και του χρόνου που δημιουργήθηκε,
						   ή αλλιώς ο συνολικός χρόνος ζωής της διεργασίας)
totalWaitingTime     -->   Ο συνολικός χρόνος αναμονής, ο οποίος προκύπτει απο τη διαφορά του χρόνου διεκπεραίωσης και του χρόνου εκτέλεσης.

Επιπλέον, τροποποιήσαμε ελάχιστα το cpu.c για λόγους καταγραφής των μετρήσεων, και προσθέσαμε τις εξής γραμμές στο killtask (πριν αφαιρεθεί και γίνει free το task):
j->totalProcessingTime = sched_clock() - j->totalProcessingTime;
j->totalWaitingTime = j->totalProcessingTime - j->totalExecutionTime;
fprintf(stderr, "Task --%s-- total processing time: %llu , total execution time: %llu , total waiting time: %llu \n", j->thread_info->processName, j->totalProcessingTime/1000000, j->totalExecutionTime/1000000, j->totalWaitingTime/1000000);

Η τιμή του totalProcessingTime αρχικοποιείται μαζί με τα άλλα πεδία στην sched_fork του schedule.c . Αυτές οι τιμές υπολογίζονται και τυπώνονται στο stderr για κάθε task εκτός της init (το ελέγχουμε με μια if).


-------Αναμενόμενα αποτελέσματα-------
Ο αλγόριθμος SJF με goodness θα πρέπει να μειώνει το waiting time των processes που έχουν μείνει για πολλή ώρα στο queue, σε αντίθεση με τον SJF με Min Exp Burst,
ο οποίος πάντα θα δίνει τον επεξεργαστή στην πιο καινούρια διεργασία, με κίνδυνο να "λιμοκτονήσουν" οι υπόλοιπες.


-------Προφίλ δοκιμών-------
Δημιουργήσαμε 7 προφίλ δοκιμών. Για κάθε προφίλ υπάρχουν λίγα λόγια για το τι κάνει, τα αποτελέσματα απο τις μετρήσεις, ο μέσος χρόνος αναμονής, καθώς και ένα σύντομο συμπέρασμα.


Τα δύο προφίλ-highlight του report είναι τα προφίλ 3 και 7. Αυτά δείχνουν το μεγάλο όφελος του goodness σε σχέση με το min exp burst.


=================================================================Profile 3================================================================
Αυτό το προφίλ προσομοιώνει ένα starvation των processes. Συγκεκριμένα, δημιουργεί ένα μεγάλο non-interactive process τη στιγμή 30 και μετά,
κάθε 10ms δημιουργεί από ένα μικρότερο non-interactive process (με μικρό workduration).

|-------------------------------------------------Results with goodness-------------------------------------------------|
|   Task --(PROC_2_NONINTERACTIVE:2)-- total processing time: 20 , total execution time: 10 , total waiting time: 10    |	
|   Task --(PROC_3_NONINTERACTIVE:3)-- total processing time: 20 , total execution time: 10 , total waiting time: 10    |
|   Task --(PROC_4_NONINTERACTIVE:4)-- total processing time: 20 , total execution time: 10 , total waiting time: 10    |
|   Task --(PROC_5_NONINTERACTIVE:5)-- total processing time: 20 , total execution time: 10 , total waiting time: 10    |
|   Task --(PROC_6_NONINTERACTIVE:6)-- total processing time: 20 , total execution time: 10 , total waiting time: 10    |
|   Task --(PROC_7_NONINTERACTIVE:7)-- total processing time: 30 , total execution time: 10 , total waiting time: 20    |
|   Task --(PROC_8_NONINTERACTIVE:8)-- total processing time: 40 , total execution time: 20 , total waiting time: 20    |
|   Task --(PROC_1_NONINTERACTIVE:1)-- total processing time: 280 , total execution time: 200 , total waiting time: 80  |
|-----------------------------------------------------------------------------------------------------------------------|

Average waiting time (goodness): 21.25 ms

|----------------------------------------------Results with min exp burst-----------------------------------------------|
|   Task --(PROC_8_NONINTERACTIVE:8)-- total processing time: 10 , total execution time: 10 , total waiting time: 0     |
|   Task --(PROC_7_NONINTERACTIVE:7)-- total processing time: 20 , total execution time: 10 , total waiting time: 10    |
|   Task --(PROC_6_NONINTERACTIVE:6)-- total processing time: 30 , total execution time: 10 , total waiting time: 20    |
|   Task --(PROC_5_NONINTERACTIVE:5)-- total processing time: 40 , total execution time: 10 , total waiting time: 30    |
|   Task --(PROC_3_NONINTERACTIVE:3)-- total processing time: 70 , total execution time: 10 , total waiting time: 60    |
|   Task --(PROC_2_NONINTERACTIVE:2)-- total processing time: 80 , total execution time: 10 , total waiting time: 70    |
|   Task --(PROC_4_NONINTERACTIVE:4)-- total processing time: 70 , total execution time: 20 , total waiting time: 50    |
|   Task --(PROC_1_NONINTERACTIVE:1)-- total processing time: 280 , total execution time: 200 , total waiting time: 80  |
|-----------------------------------------------------------------------------------------------------------------------|

Average waiting time (min exp burst): 40 ms

Παρατηρούμε πως πράγματι, στον SJF με min exp burst, οι διεργασίες που μπήκαν νωρίς στο queue, αναγκάζονται να περιμένουν πολύ περισσότερο απο αυτές που μπήκαν αργότερα.
Στον SJF με goodness ο μέσος χρόνος αναμονής μειώνεται σχεδόν στο μισό.

==========================================================================================================================================


=================================================================Profile 7================================================================
Αυτό το προφίλ προσομοιώνει επίσης ένα starvation των processes. Συγκεκριμένα, δημιουργεί ένα μεγάλο interactive process τη στιγμή 20 και μετά,
κάθε 10ms δημιουργεί από ένα μικρότερο non-interactive process (με μικρό workduration).

|-------------------------------------------------Results with goodness-------------------------------------------------|
|   Task --(PROC_2_NONINTERACTIVE:2)-- total processing time: 20 , total execution time: 10 , total waiting time: 10    |
|   Task --(PROC_3_NONINTERACTIVE:3)-- total processing time: 20 , total execution time: 10 , total waiting time: 10    |
|   Task --(PROC_4_NONINTERACTIVE:4)-- total processing time: 20 , total execution time: 10 , total waiting time: 10    |
|   Task --(PROC_5_NONINTERACTIVE:5)-- total processing time: 20 , total execution time: 10 , total waiting time: 10    |
|   Task --(PROC_6_NONINTERACTIVE:6)-- total processing time: 30 , total execution time: 10 , total waiting time: 20    |
|   Task --(PROC_7_NONINTERACTIVE:7)-- total processing time: 40 , total execution time: 20 , total waiting time: 20    |
|   Task --(PROC_1_INTERACTIVE:1)-- total processing time: 270 , total execution time: 200 , total waiting time: 70     |
|-----------------------------------------------------------------------------------------------------------------------|

Average waiting time (goodness): 21.4 ms

|---------------------------------------------Results with min exp burst------------------------------------------------|
|   Task --(PROC_7_NONINTERACTIVE:7)-- total processing time: 10 , total execution time: 10 , total waiting time: 0     |
|   Task --(PROC_6_NONINTERACTIVE:6)-- total processing time: 20 , total execution time: 10 , total waiting time: 10    |
|   Task --(PROC_5_NONINTERACTIVE:5)-- total processing time: 30 , total execution time: 10 , total waiting time: 20    |
|   Task --(PROC_4_NONINTERACTIVE:4)-- total processing time: 40 , total execution time: 10 , total waiting time: 30    |
|   Task --(PROC_2_NONINTERACTIVE:2)-- total processing time: 70 , total execution time: 10 , total waiting time: 60    |
|   Task --(PROC_3_NONINTERACTIVE:3)-- total processing time: 70 , total execution time: 20 , total waiting time: 50    |
|   Task --(PROC_1_INTERACTIVE:1)-- total processing time: 270 , total execution time: 200 , total waiting time: 70     |
|-----------------------------------------------------------------------------------------------------------------------|

Average waiting time (min exp burst): 34.2 ms

Και εδώ παρατηρούμε πως στον SJF με min exp burst, οι διεργασίες που μπήκαν νωρίς στο queue, αναγκάζονται να περιμένουν πολύ περισσότερο απο αυτές που μπήκαν αργότερα.
Στον SJF με goodness ο μέσος χρόνος αναμονής μειώνεται κατά πολύ.

==========================================================================================================================================


Ακολουθούν τα υπόλοιπα προφίλ δοκιμών


=================================================================Profile 1================================================================
Το προφίλ δημιουργεί non-interactive processes σε διάφορα χρονικά διαστήματα και με διάφορα workduration.

|---------------------------------------------------Results with goodness---------------------------------------------------|
|   Task --(PROC_2_NONINTERACTIVE:2)-- total processing time: 840 , total execution time: 200 , total waiting time: 640     |
|   Task --(PROC_3_NONINTERACTIVE:3)-- total processing time: 850 , total execution time: 200 , total waiting time: 650     |
|   Task --(PROC_1_NONINTERACTIVE:1)-- total processing time: 910 , total execution time: 230 , total waiting time: 680     |
|   Task --(PROC_4_NONINTERACTIVE:4)-- total processing time: 1000 , total execution time: 250 , total waiting time: 750    |
|   Task --(PROC_5_NONINTERACTIVE:5)-- total processing time: 1000 , total execution time: 270 , total waiting time: 730    |
|---------------------------------------------------------------------------------------------------------------------------|

Average waiting time (goodness): 690 ms

|------------------------------------------------Results with min exp burst-------------------------------------------------|
|   Task --(PROC_3_NONINTERACTIVE:3)-- total processing time: 860 , total execution time: 180 , total waiting time: 680     |
|   Task --(PROC_1_NONINTERACTIVE:1)-- total processing time: 930 , total execution time: 180 , total waiting time: 750     |
|   Task --(PROC_2_NONINTERACTIVE:2)-- total processing time: 980 , total execution time: 200 , total waiting time: 780     |
|   Task --(PROC_4_NONINTERACTIVE:4)-- total processing time: 970 , total execution time: 250 , total waiting time: 720     |
|   Task --(PROC_5_NONINTERACTIVE:5)-- total processing time: 960 , total execution time: 300 , total waiting time: 660     |
|---------------------------------------------------------------------------------------------------------------------------|

Average waiting time (min exp burst): 718 ms

Παρατηρούμε βελτίωση του μέσου χρόνου αναμονής στον SJF με goodness, όπως είναι αναμενόμενο.

==========================================================================================================================================


=================================================================Profile 2================================================================
Το προφίλ δημιουργεί 5 non-interactive processes, όλα την στιγμή 50.

|---------------------------------------------------Results with goodness---------------------------------------------------|
|   Task --(PROC_1_NONINTERACTIVE:1)-- total processing time: 950 , total execution time: 230 , total waiting time: 720     |
|   Task --(PROC_2_NONINTERACTIVE:2)-- total processing time: 950 , total execution time: 230 , total waiting time: 720     |
|   Task --(PROC_5_NONINTERACTIVE:5)-- total processing time: 950 , total execution time: 120 , total waiting time: 830     |
|   Task --(PROC_3_NONINTERACTIVE:3)-- total processing time: 950 , total execution time: 230 , total waiting time: 720     |
|   Task --(PROC_4_NONINTERACTIVE:4)-- total processing time: 950 , total execution time: 140 , total waiting time: 810     |
|---------------------------------------------------------------------------------------------------------------------------|

Average waiting time (goodness): 760 ms

|------------------------------------------------Results with min exp burst---------------------------------------------|
|   Task --(PROC_5_NONINTERACTIVE:5)-- total processing time: 950 , total execution time: 190 , total waiting time: 760 |
|   Task --(PROC_4_NONINTERACTIVE:4)-- total processing time: 950 , total execution time: 190 , total waiting time: 760 |
|   Task --(PROC_2_NONINTERACTIVE:2)-- total processing time: 950 , total execution time: 190 , total waiting time: 760 |
|   Task --(PROC_1_NONINTERACTIVE:1)-- total processing time: 950 , total execution time: 190 , total waiting time: 760 |
|   Task --(PROC_3_NONINTERACTIVE:3)-- total processing time: 950 , total execution time: 190 , total waiting time: 760 |
|-----------------------------------------------------------------------------------------------------------------------|

Average waiting time (min exp burst): 760 ms

Παρατηρούμε ίδιο μέσο χρόνο αναμονής, αφού οι διεργασίες είναι όλες non-interactive και δημιουργούνται την ίδια στιγμή.

==========================================================================================================================================


=================================================================Profile 4================================================================
Το προφίλ δημιουργεί 5 interactive processes, όλα την στιγμή 50.

|---------------------------------------------------Results with goodness-----------------------------------------------|
|   Task --(PROC_1_INTERACTIVE:1)-- total processing time: 950 , total execution time: 390 , total waiting time: 560	|
|   Task --(PROC_3_INTERACTIVE:3)-- total processing time: 1160 , total execution time: 90 , total waiting time: 1070	|
|   Task --(PROC_2_INTERACTIVE:2)-- total processing time: 1160 , total execution time: 250 , total waiting time: 910	|
|   Task --(PROC_4_INTERACTIVE:4)-- total processing time: 1160 , total execution time: 220 , total waiting time: 940	|
|-----------------------------------------------------------------------------------------------------------------------|

Average waiting time (goodness): 870 ms

|------------------------------------------------Results with min exp burst---------------------------------------------|
|   Task --(PROC_3_INTERACTIVE:3)-- total processing time: 950 , total execution time: 440 , total waiting time: 510    |
|   Task --(PROC_4_INTERACTIVE:4)-- total processing time: 1160 , total execution time: 150 , total waiting time: 1010  |
|   Task --(PROC_2_INTERACTIVE:2)-- total processing time: 1160 , total execution time: 160 , total waiting time: 1000  |
|   Task --(PROC_1_INTERACTIVE:1)-- total processing time: 1160 , total execution time: 200 , total waiting time: 960   |
|-----------------------------------------------------------------------------------------------------------------------|

Average waiting time (min exp burst): 870 ms

Όπως στον προφίλ 2, παρατηρούμε ίδιο μέσο χρόνο αναμονής.

==========================================================================================================================================


=================================================================Profile 5================================================================
Το προφίλ δημιουργεί interactive processes με διάφορα workdurations, καθώς και spawners.

|---------------------------------------------------------Results with goodness---------------------------------------------------------|
|   Task --(PROC_2_INTERACTIVE:2)-- total processing time: 530 , total execution time: 100 , total waiting time: 430                    |
|   Task --(PROC_4_INTERACTIVE:7)-- total processing time: 1180 , total execution time: 210 , total waiting time: 970                   |
|   Task --(PROC_1_SPAWNER:1):(PROC_1_CHILD_2:5)-- total processing time: 1660 , total execution time: 320 , total waiting time: 1340   |
|   Task --(PROC_1_SPAWNER:1):(PROC_1_CHILD_1:4)-- total processing time: 1700 , total execution time: 220 , total waiting time: 1480   |
|   Task --(PROC_1_SPAWNER:1)-- total processing time: 1770 , total execution time: 240 , total waiting time: 1530                      |
|   Task --(PROC_3_SPAWNER:3):(PROC_3_CHILD_1:6)-- total processing time: 1620 , total execution time: 380 , total waiting time: 1240   |
|   Task --(PROC_3_SPAWNER:3)-- total processing time: 1750 , total execution time: 300 , total waiting time: 1450                      |
|---------------------------------------------------------------------------------------------------------------------------------------|

Average waiting time (goodness): 1205 ms

|-----------------------------------------------------Results with min exp burst--------------------------------------------------------|
|   Task --(PROC_2_INTERACTIVE:2)-- total processing time: 720 , total execution time: 100 , total waiting time: 620                    |
|   Task --(PROC_4_INTERACTIVE:7)-- total processing time: 1150 , total execution time: 190 , total waiting time: 960                   |
|   Task --(PROC_1_SPAWNER:1):(PROC_1_CHILD_2:5)-- total processing time: 1950 , total execution time: 410 , total waiting time: 1540   |
|   Task --(PROC_1_SPAWNER:1):(PROC_1_CHILD_1:4)-- total processing time: 2010 , total execution time: 250 , total waiting time: 1760   |
|   Task --(PROC_3_SPAWNER:3):(PROC_3_CHILD_1:6)-- total processing time: 2240 , total execution time: 270 , total waiting time: 1970   |
|   Task --(PROC_1_SPAWNER:1)-- total processing time: 2410 , total execution time: 400 , total waiting time: 2010                      |
|   Task --(PROC_3_SPAWNER:3)-- total processing time: 2380 , total execution time: 520 , total waiting time: 1860                      |
|---------------------------------------------------------------------------------------------------------------------------------------|

Average waiting time (min exp burst): 1531 ms

Παρατηρούμε βελτίωση του μέσου χρόνου αναμονής στον SJF με goodness, όπως είναι αναμενόμενο.

==========================================================================================================================================


=================================================================Profile 6================================================================
Το προφίλ δημιουργεί interactive και non-interactive processes, με διάφορα workdurations, καθώς και spawners. Είναι η περίπτωση του mixed.

|---------------------------------------------------------Results with goodness---------------------------------------------------------|
|   Task --(PROC_3_SPAWNER:3):(PROC_3_CHILD_2:5)-- total processing time: 490 , total execution time: 50 , total waiting time: 440      |
|   Task --(PROC_5_NONINTERACTIVE:7)-- total processing time: 490 , total execution time: 50 , total waiting time: 440                  |
|   Task --(PROC_7_INTERACTIVE:9)-- total processing time: 510 , total execution time: 50 , total waiting time: 460                     |
|   Task --(PROC_2_NONINTERACTIVE:2)-- total processing time: 640 , total execution time: 70 , total waiting time: 570                  |
|   Task --(PROC_7_NONINTERACTIVE:10)-- total processing time: 500 , total execution time: 50 , total waiting time: 450                 |
|   Task --(PROC_1_NONINTERACTIVE:1)-- total processing time: 790 , total execution time: 100 , total waiting time: 690                 |
|   Task --(PROC_4_INTERACTIVE:6)-- total processing time: 770 , total execution time: 100 , total waiting time: 670                    |
|   Task --(PROC_6_SPAWNER:8):(PROC_6_CHILD_1:11)-- total processing time: 2320 , total execution time: 310 , total waiting time: 2010  |
|   Task --(PROC_3_SPAWNER:3):(PROC_3_CHILD_1:4)-- total processing time: 9950 , total execution time: 2720 , total waiting time: 7230  |
|   Task --(PROC_6_SPAWNER:8)-- total processing time: 9890 , total execution time: 2190 , total waiting time: 7700                     |
|   Task --(PROC_3_SPAWNER:3)-- total processing time: 9990 , total execution time: 2460 , total waiting time: 7530                     |
|---------------------------------------------------------------------------------------------------------------------------------------|

Average waiting time (goodness): 2562 ms

|-----------------------------------------------------Results with min exp burst--------------------------------------------------------|
|   Task --(PROC_7_NONINTERACTIVE:10)-- total processing time: 400 , total execution time: 50 , total waiting time: 350                 |
|   Task --(PROC_7_INTERACTIVE:9)-- total processing time: 420 , total execution time: 50 , total waiting time: 370                     |
|   Task --(PROC_5_NONINTERACTIVE:7)-- total processing time: 500 , total execution time: 50 , total waiting time: 450                  |
|   Task --(PROC_3_SPAWNER:3):(PROC_3_CHILD_2:6)-- total processing time: 500 , total execution time: 50 , total waiting time: 450      |
|   Task --(PROC_2_NONINTERACTIVE:2)-- total processing time: 710 , total execution time: 70 , total waiting time: 640                  |
|   Task --(PROC_4_INTERACTIVE:4)-- total processing time: 810 , total execution time: 100 , total waiting time: 710                    |
|   Task --(PROC_1_NONINTERACTIVE:1)-- total processing time: 870 , total execution time: 100 , total waiting time: 770                 |
|   Task --(PROC_6_SPAWNER:8):(PROC_6_CHILD_1:11)-- total processing time: 3710 , total execution time: 280 , total waiting time: 3430  |
|   Task --(PROC_3_SPAWNER:3):(PROC_3_CHILD_1:5)-- total processing time: 10300 , total execution time: 1890 , total waiting time: 8410 |
|   Task --(PROC_6_SPAWNER:8)-- total processing time: 10280 , total execution time: 2340 , total waiting time: 7940                    |
|   Task --(PROC_3_SPAWNER:3)-- total processing time: 10380 , total execution time: 3160 , total waiting time: 7220                    |
|---------------------------------------------------------------------------------------------------------------------------------------|

Average waiting time (min exp burst): 2794 ms

Παρατηρούμε βελτίωση του μέσου χρόνου αναμονής στον SJF με goodness, όπως είναι αναμενόμενο.

==========================================================================================================================================


=======ΓΕΝΙΚΑ ΣΧΟΛΙΑ ΓΙΑ ΤΑ ΠΡΟΦΙΛ=======
Κατά τις εκτελέσεις, παρατηρήσαμε πως, είτε όταν έχουμε μόνο 2 processes στο queue, ή όταν όλα τα processes δημιουργούνται την ίδια στιγμή,
οι αλγόριθμοι συμπεριφέρονται πολύ παρόμοια με τον Round Robin, δηλαδή κάνουν "cycle" μεταξύ των processes.



----------Υποσημείωση για τους μετρημένους χρόνους----------
Όλοι οι χρόνοι που μετρήθηκαν, είναι πολλαπλάσιοι του 10. Αυτό γιατί οι αριθμοί ανανεώνονται κάθε φορά που γίνεται reschedule,
το οποίο στην περίπτωσή μας είναι κάθε 10ms. Για παράδειγμα, μία διεργασία που έχει χρόνο εκτέλεσης 12ms, θα μετρηθεί όταν μπει στον scheduler στα 10ms, μετά στα 12ms
θα "πεθάνει", επομένως όταν ξαναμπεί στον scheduler θα έχει ήδη γίνει free και δεν θα έχουν μετρηθεί αυτά τα 2 έξτρα δευτερόλεπτα (θα μπορούσαμε να τα μετρήσουμε στο cpu.c,
όμως δεν το κάναμε σε αυτή την υλοποίηση). Προφανώς, στην υλοποίηση του αλγορίθμου δεν παίζει κανένα λόγο διότι δεν μας νοιάζουν διεργασίες που έχουν φύγει.
Αυτό το φαινόμενο δεν επηρεάζει ούτε τις μετρήσεις επειδή, πρώτον συμβαίνει σε όλες τις διεργασιες και δεύτερον συμβαίνει μόνο μια φορά,
πριν "πεθάνει" η διεργασία. Έτσι τα νούμερα έχουν μικρή απόκλιση που δεν επηρεάζει τη συνολική εικόνα σύγκρισης των 2 αλγορίθμων, που είναι ο στόχος μας.