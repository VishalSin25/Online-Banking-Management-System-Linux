/* Implementing Client for the Banking Management System.
Name: Subham Basu Roy Chowdhury
Roll: MT2022118
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "Customer.h"
#include "Account.h"
#include "Transaction.h"

void error(const char *msg);
void addAcc(int add_sd,struct Customer login);
void delAcc(int sock,struct Customer login);
void search(int sock,struct Customer login);
void deposit(int sock,struct Customer login);
void withdraw(int sock,struct Customer login);
void viewDetails(int sock,struct Customer login);
void changePwd(int sock,struct Customer cust);
void userMenu(int sock,struct Customer login);
void adminMenu(int admin_sd,struct Customer login);

void error(const char *msg){
    perror(msg);
    exit(1);
}

void addAcc(int add_sd,struct Customer login){  //called from admin menu.
    char buff[512]="";
    int buf_cust,buf_acc;
    struct Customer customer;
    int flag = 1;
    int age;
    int k;
    while(flag){
        printf("Enter the username: ");
        scanf("%s",customer.username);
        write(add_sd,&customer,sizeof(customer));   //send the username to the server.
        read(add_sd,&flag,sizeof(int));     //receive the flag value from the server.
        if(flag==1)
        {
            printf("This username already exists. Try another username\n\n\n");
        }
    }

    char password[512]="";
    printf("Enter the password: ");
    scanf("%s", password);
    write(add_sd,password,sizeof(password));

    printf("Enter the age: ");
    scanf("%d", &age);    
    int flagAge = 1;
    while(flagAge && (age <= 0 || age > 100 ))
    {
        if(age <= 0 || age > 100)
        {
            printf("\n Please enter a valid age \n");
            printf("\nReEnter age: ");
            scanf("%d",&age);
        }
        else    
            flagAge = 0;
    }
    write(add_sd,&age,sizeof(age));
    
    read(add_sd,&customer,sizeof(customer));
    printf("\nWelcome %s, your account is created with account number: %ld and available balance: 0.00\n\n",customer.username, customer.account_number);
    printf("Press 1 to continue\n");
    scanf("%d",&k);
    write(add_sd,&k,sizeof(k));
    if(k==1)
        adminMenu(add_sd,login);
}


void delAcc(int sock,struct Customer login){
    char name[512];
    char buf[100];
    int k;
    printf("Enter the username: ");
    scanf("%s",name);
    write(sock,name,sizeof(name));  //send the username to the server.
    read(sock,buf,sizeof(buf)); //if invalid username found.
    printf("%s\n",buf);
    printf("\nPress 1 to continue\n");
    scanf("%d",&k);
    write(sock,&k,sizeof(k));
    if(k==1)
        adminMenu(sock,login);
}

void search(int sock,struct Customer login){
    char name[512];
    int k;
    struct Customer customer;
    struct Account acc;
    printf("Enter the username: ");
    scanf("%s",name);

    write(sock,name,sizeof(name));  //send the username to the server.

    char msg[100];
    read(sock,&msg,sizeof(msg));    //check if Account not found sent by the server.
    if(!strcmp(msg,"Account not found!")){
        printf("No account found with user name: %s\n\n",name);
        printf("Press 1 to continue\n");
	    scanf("%d",&k);
	    write(sock,&k,sizeof(k));
	    if(k==1)    //reopen the admin Menu.
	        adminMenu(sock,login);
    }
    else{
        read(sock,&customer,sizeof(customer));  //get the customer details.
        printf("The details of the user: %s\n",customer.username);
        printf("Account Number= %ld\n",customer.account_number);
        printf("Age= %d\n",customer.age);
        if(customer.type == false)
        {
		    printf("Type= NORMAL USER\n"); 
        }
		else
        {
		    printf("Type= ADMIN\n"); 
        }
		
		if(customer.status == true)
        {
            printf("Status= ACTIVE\n"); 
        }
		else
        {
		    printf("Status= INACTIVE\n"); 
        }
        
        read(sock,&acc,sizeof(acc));    //get the account details.
        printf("Account Balance= %.2f\n",acc.balance); 
        printf("\nPress 1 to continue\n");
	    scanf("%d",&k);
	    write(sock,&k,sizeof(k));
	    if(k==1)
	        adminMenu(sock,login);
    }
}

void deposit(int sock,struct Customer login){
    double amount;
    double prev_bal,new_bal;
    int k,u;
    printf("\nEnter amount: ");
    scanf("%lf",&amount);
    int flag = 1;
    while(flag && (amount<0))
    {
        if(amount < 0)
        {
            printf("\n Please enter an amount >= 0\n");
            printf("\nEnter amount: ");
            scanf("%lf",&amount);
        }
        else    
            flag = 0;
    }

    write(sock, &amount, sizeof(amount)); //send amount to server.
    read(sock, &prev_bal, sizeof(prev_bal));    //get prev balance
    printf("Previous balance = %.2lf\n",prev_bal);
    read(sock, &new_bal, sizeof(new_bal));  //get updated balance.
    printf("Updated balance = %.2lf\n",new_bal);
    printf("\nPress 0 to unlock\n");
    scanf("%d",&u);
    write(sock,&u,sizeof(u));       //send 0.
	printf("\nPress 1 to continue\n");
    scanf("%d",&k);
    write(sock,&k,sizeof(k));
    if(k==1)
        userMenu(sock,login);
}


void withdraw(int sock,struct Customer login){  //call from the user menu.
    double amount;
    double prev_bal,new_bal;
    int k,u;
    printf("\nEnter amount: ");
    scanf("%lf",&amount);
    int flag = 1;
    while(flag && (amount<0))
    {
        if(amount < 0)
        {
            printf("\n Please enter an amount >= 0\n");
            printf("\nEnter amount: ");
            scanf("%lf",&amount);
        }
        else    
            flag = 0;
    }

    write(sock, &amount, sizeof(amount));  //send the amount value.
    read(sock, &prev_bal, sizeof(prev_bal)); //get the previous balance.
    printf("Previous balance = %.2lf\n",prev_bal);
    if(prev_bal<amount){
        printf("Insufficient Balance! Can't withdraw\n\n");
    }
    else{
        read(sock, &new_bal, sizeof(new_bal));  //get the new balance.
        printf("Updated balance = %.2lf\n",new_bal);
    }
    printf("\nPress 0 to unlock\n");
    scanf("%d",&u);
    write(sock,&u,sizeof(u));
    printf("\nPress 1 to continue\n");
    scanf("%d",&k);
    write(sock,&k,sizeof(k));
    if(k==1)
        userMenu(sock,login);
}

void passBook(int sock,struct Customer login){  //we are updating the details of the transaction in the trans db.
    struct Transaction trans;
    int k;
    read(sock,&trans,sizeof(trans));    //get the transaction details.
    printf("Transaction Date   = %s",trans.date);
    printf("Transaction Amount = %.2f\n",trans.amount);
    if(trans.type == false){
        printf("Transaction Type = CREDIT\n");
    }
    else{
        printf("Transaction Type = DEBIT\n");
    }
    printf("Avaiable Balance   = %.2f\n",trans.balance);
    printf("\nPress 1 to continue\n");
    scanf("%d",&k);
    write(sock,&k,sizeof(k));
    if(k==1)
        userMenu(sock,login); 
}

void viewDetails(int sock,struct Customer login){
    struct Account account;
    struct Customer customer;
    int k;
    read(sock,&account,sizeof(account)); //get accnt details.
    read(sock,&customer,sizeof(customer)); //get cust details.
    printf("Username:                     %s\n",customer.username);
    printf("Password:                     %s\n",customer.password);
    if(customer.status == true){
    printf("Status:                       ACTIVE\n"); }
	else{
	printf("Status:                       INACTIVE\n"); }
    printf("Account number:               %ld\n",account.account_number);
    printf("Current Balance:              %.2lf\n",account.balance);
	printf("\nPress 1 to continue\n");
    scanf("%d",&k);
    write(sock,&k,sizeof(k));
    if(k==1)
        userMenu(sock,login);

}


void changePwd(int sock,struct Customer cust){
    printf("Press 1 to continue\n");
    int n,k;
    scanf("%d",&n);
    char pass[512]="";
    char new_pass[512]="";
    char new_pass_confirm[512]="";
    if(n==1)
    {
        printf("Enter current password: ");
        scanf("%s",pass);
        write(sock,&pass,sizeof(pass)); //send the current password.
        int n;
        read(sock,&n,sizeof(n));    //if n == 0 incorrect password entered and checked in server.
        if(n==0)
        {
            printf("Pls. enter correct password\n\n\n");
        }
        else if(n==1)
        {
            printf("Enter new Password: ");
            scanf("%s",new_pass);
            printf("Confirm new Password: ");
            scanf("%s",new_pass_confirm);
            if(!strcmp(new_pass_confirm,new_pass))
            {
                write(sock,&new_pass,sizeof(new_pass)); //send new password  to server.
                printf("Password updated successfully!!\n\n");
            }
            else{
                printf("Couldn't confirm new Password\n\n\n");
                
            }
        }        
    }
    else{
        printf("Invalid Input. Try Again\n\n\n");
    }
    printf("Press 1 to continue\n");
    scanf("%d",&k);
    write(sock,&k,sizeof(k));
    if(k==1)
        userMenu(sock,cust);
}

void userMenu(int sock,struct Customer login){
    int choice;
    printf("Choose an Option\n\n");
    printf("Type 1 to Deposit some amount\n");
    printf("Type 2 to Withdraw some amount\n");
    printf("Type 3 to View account details\n");
    printf("Type 4 to Change password\n");
    printf("Type 5 to View last transaction\n");
    printf("Hit any other key to log out...\n");

    printf("\nEnter Your Choice: ");
    scanf("%d", &choice);
    if(choice > 5 || choice < 1){
        write(sock, &choice, sizeof(choice));   //give input to server and get break in default case and exit.
        printf("\n\nLogging out...\n\n");
        return;
    }
    write(sock, &choice, sizeof(choice)); //valid choice. send it
    int n;
    switch(choice){
        case 1: 
            deposit(sock,login);
            break;
        case 2: 
            withdraw(sock,login);
            break;
        case 3: 
            viewDetails(sock,login);
            break;
        case 4: 
            changePwd(sock,login);
            break;
        case 5: 
            passBook(sock,login);
            break;
        default:
            break;
    }
}

void adminMenu(int admin_sd,struct Customer login){ //called from the main function.
    int choice;
    printf("Choose an Option: \n\n");
    printf("Type 1 to Add an account\n");
    printf("Type 2 to Delete an account\n");
    printf("Type 3 to Search for an account\n");
    printf("Hit any other key to log out...\n");

    printf("\nEnter your choice: ");
    scanf("%d", &choice);


    if(choice >= 4 || choice < 1){
        write(admin_sd, &choice, sizeof(choice)); //if invalid choice provided write to the socket and return with break from the adminMenu in the server and exit the code.
        printf("\n\nLogging out...\n\n");
        return;                    
    }
    write(admin_sd, &choice, sizeof(choice)); //for valid input.
    int n;
    switch(choice){
        case 1: 
            addAcc(admin_sd,login);
            break;
        case 2: 
            delAcc(admin_sd,login); 
            break;
            
        case 3: 
            search(admin_sd,login); 
            break;
        
        default: 
            break;
    }
}


int main(){
    struct Customer customer;
    struct sockaddr_in server;
    int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(46316);
    connect(sock, (void *)(&server), sizeof(server));
    printf("\n::::::::::::::::::::::::::  WELCOME TO ONLINE BANKING MANAGEMENT SYSTEM  ::::::::::::::::::::::::::\n");
    printf("\nEnter username: ");
    scanf("%s", customer.username);
    printf("Enter password: ");
    scanf("%s", customer.password);

    write(sock, &customer, sizeof(customer));   //write into the socket and send it to the server for the function authorize.
    char buff[512]="";
    int validate=read(sock, buff, sizeof(buff)); //output of the authorize function from the server. (validation successful or validation failed message.)
    if(!strcmp(buff,"Validation successful")){
        if(validate>=0 && strcmp(customer.username,"admin")==0 ){
            printf("\n\nWELCOME ADMIN\n\n");
    		adminMenu(sock,customer);
        }
        else{
            printf("\n\nWELCOME %s\n\n",customer.username);
    		userMenu(sock,customer);
        }
    }
    return 0;
}
