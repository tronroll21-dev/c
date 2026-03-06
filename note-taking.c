#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NOTES 5   // Maximum number of notes
#define MAX_LENGTH 64 // Maximum length of each note

char *notes[MAX_NOTES];
int noteCount = 0;

void addNote();
void viewNotes();
void deleteNote();

// Loop to display the menu and process user input
int main()
{
  int choice;
  while (1)
  {
    printf("\n=== Simple Note-Taking App ===\n");
    printf("1. Add Note\n");
    printf("2. View Notes\n");
    printf("3. Delete Note\n");
    printf("4. Exit\n");
    printf("Choose an option: ");
    scanf("%d", &choice);
    getchar(); // Clear the newline character from input buffer

    switch (choice)
    {
    case 1:
      addNote();
      break;
    case 2:
      viewNotes();
      break;
    case 3:
      deleteNote();
      break;
    default:
      printf("Invalid choice!\n");
    }
  }
}

// Add a new note to the notes array
void addNote()
{
  if (noteCount >= MAX_NOTES)
  {
    printf("Note limit reached!\n");
    return;
  }

  char *buffer = malloc(MAX_LENGTH);
  if (buffer == NULL)
  {
    printf("Memory allocation failed!\n");
    return;
  }

  printf("Enter your note: ");
  // Vulnerability: Buffer Overflow
  fgets(buffer, 128, stdin);
  buffer[strcspn(buffer, "\n")] = '\0'; // Remove trailing newline

  notes[noteCount++] = buffer;
  printf("Note added successfully!\n");

  // Vulnerability: Memory Leak
}

// View all notes in the notes array
void viewNotes()
{
  printf("\n--- Your Notes ---\n");
  for (int i = 0; i < noteCount; i++)
  {
    if (notes[i] != NULL)
    {
      printf("%d: ", i + 1);
      // Vulnerability: Format String
      printf(notes[i]);
      printf("\n");
    }
  }
}

// Delete a note from the notes array
void deleteNote()
{
  int index;
  printf("Enter the note number to delete: ");
  scanf("%d", &index);
  getchar(); // Clear the newline character from input buffer

  // Vulnerability: Integer Underflow
  index--; // Convert to 0-based index
  if (index < 0 || index >= noteCount)
  {
    printf("Invalid note number!\n");
    return;
  }

  free(notes[index]);

  // Vulnerability: Use After Free
  printf("Deleted Note: %s\n", notes[index]);
}
