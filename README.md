
# Virtual Memory Allocator - Tema 1

**Nume:** Potoceanu Ana-Maria  


## Descrierea Temei

Pentru implementarea alocatorului de memorie, am avut nevoie de structurile necesare pentru o listă înlănțuită, pentru un nod care va avea doi pointeri: `next` și `prev`, dar și de cele oferite în scheletul temei.

În funcția `alloc_arena`, am realizat inițializarea arenei și am creat cu ajutorul funcției `create_list` lista conținută de arenă. În funcția `alloc_block`, am tratat problema în trei cazuri principale:
- Primul caz apare când lista dublu înlănțuită din arenă nu conține niciun element, astfel se creează și se adaugă un block nou pe poziția 0 în listă, utilizând funcția `create_block`.
- În al doilea caz, avem un singur block în listă, unde se verifică validitatea adresei primite și se decide locul de inserare pentru block sau miniblock în funcție de ocuparea zonelor de memorie.
- În ultimul caz, când lista conține două sau mai multe block-uri, am tratat cazurile de adăugare la începutul listei, la final, sau între două block-uri existente.

Funcția `dealloc_arena` eliberează întreaga memorie alocată pentru fiecare block, miniblock, și lista dublu înlănțuită. Aceasta finalizează prin eliberarea memoriei arenei.

Funcția `free_block` gestionează eliberarea unui miniblock, tratând patru cazuri posibile pentru miniblock-ul vizat, inclusiv cazurile când există doar un miniblock sau miniblock-ul este la început, la final, ori la mijlocul listei.

În funcția `pmap`, am parcurs toate block-urile și miniblock-urile pentru a afișa informațiile cerute, utilizând o funcție `per` pentru generarea permisiunilor în format de string, de exemplu, "RWX" pentru 7.

Funcțiile `write` și `read` sunt realizate pentru manipularea conținutului de memorie. `write` folosește `memcpy` și verifică dacă scrierea începe de la începutul miniblock-ului sau de la o adresă specifică, iar `read` funcționează similar, având grijă de permisiuni și cazuri multiple de miniblock-uri.

Funcția `mprotect` schimbă permisiunile în funcție de argumentele primite: READ adaugă 4, WRITE 2, iar EXEC 1.

Această temă m-a ajutat să înțeleg funcționarea listelor înlănțuite și să implementez o structură de date generică, aprofundând cunoștințe importante pentru viitor.
