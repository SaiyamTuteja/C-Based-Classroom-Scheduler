#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

// Simple test function to demonstrate WhatsApp sharing
void testWhatsAppSharing() {
    printf("=== WhatsApp Sharing Test ===\n");
    
    // Simulate a timetable link
    const char* testLink = "https://catbox.moe/example_timetable.txt";
    
    printf("Generated timetable link: %s\n", testLink);
    printf("\nWould you like to:\n");
    printf("1. Share this link to WhatsApp group\n");
    printf("2. Exit\n");
    printf("Enter choice: ");
    
    int choice;
    scanf("%d", &choice);
    
    if (choice == 1) {
        // Create message with the timetable link
        char message[1024];
        snprintf(message, sizeof(message), 
            "📅 *MCA Timetable Update*\n\n"
            "Hello everyone! Here's the updated timetable:\n"
            "%s\n\n"
            "You can download the complete timetable file from this link. "
            "Please share this with your classmates! 📚✨",
            testLink);
        
        // URL encode the message (simplified version)
        char encodedMessage[2048];
        // Simple URL encoding for demo
        int j = 0;
        for (int i = 0; message[i] != '\0' && j < sizeof(encodedMessage) - 1; i++) {
            if (message[i] == ' ') {
                encodedMessage[j++] = '%';
                encodedMessage[j++] = '2';
                encodedMessage[j++] = '0';
            } else if (message[i] == '\n') {
                encodedMessage[j++] = '%';
                encodedMessage[j++] = '0';
                encodedMessage[j++] = 'A';
            } else {
                encodedMessage[j++] = message[i];
            }
        }
        encodedMessage[j] = '\0';
        
        // Create WhatsApp Web URL with pre-filled message
        char whatsappUrl[3072];
        snprintf(whatsappUrl, sizeof(whatsappUrl), 
            "https://wa.me/?text=%s", encodedMessage);
        
        printf("\nOpening WhatsApp Web with pre-filled message...\n");
        printf("URL: %s\n", whatsappUrl);
        
        // Open WhatsApp Web in browser
        char command[3200];
        snprintf(command, sizeof(command), "start \"\" \"%s\"", whatsappUrl);
        system(command);
        
        printf("\nWhatsApp Web should now open in your browser.\n");
        printf("The message is pre-filled with the timetable link.\n");
        printf("You can now select the MCA group or any contact to send it to.\n");
        
        // Also provide the group invite link
        printf("\nMCA Group Link: https://chat.whatsapp.com/KJZLZnetrNW4rDr4N8eKYi\n");
        
        char groupCommand[512];
        snprintf(groupCommand, sizeof(groupCommand), "start \"\" \"https://chat.whatsapp.com/KJZLZnetrNW4rDr4N8eKYi\"");
        printf("Opening group invite link...\n");
        system(groupCommand);
    }
    
    printf("\nTest completed!\n");
}

int main() {
    testWhatsAppSharing();
    return 0;
} 