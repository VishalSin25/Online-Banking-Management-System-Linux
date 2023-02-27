/* Implementing Server for the Banking Management System.
Name: Subham Basu Roy Chowdhury
Roll: MT2022118
*/

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "Customer.h"
#include "Account.h"
#include "Transaction.h"

void connect_admin();
void addAcc(int sock,struct Customer login);
int checkAcc(char name[]);
void delAcc(int sock,struct Customer login);
void search(int sock,struct Customer login);
void adminMenu(int sock,struct Customer login);
void deposit(int sock,struct Customer acc);
void withdraw(int sock, struct Customer cust);
void viewDetails(int sock,struct Customer cust);
void changePwd(int sock,struct Customer cust);
void userMenu(int sock,struct Customer login);
void authorize(int sock);


void connect_admin(){
    int fd = open("customer_db", O_CREAT|O_EXCL|O_RDWR, 0755);
    open("account_db", O_CREAT|O_EXCL|O_RDWR, 0755);
    open("transaction_db", O_CREAT|O_EXCL|O_RDWR, 0755);
    if(fd != -1){
        struct Customer admin;
        strcpy(admin.username, "admin");
        strcpy(admin.password, "1234");
        admin.type = true;
        admin.status = true;
        admin.age = 26;
        int x = write(fd, &admin, sizeof(admin));
        if(x == -1)
            printf("Error : write()");
        close(fd);
    }
}

void addAcc(int sock,struct Customer login){    //called from the admin menu.
    int fd_cust=open("customer_db", O_RDONLY);
    struct Customer cust_add;
    struct Customer record;
    int k;
    int flag = 1;
    while(flag){
        read(sock,&cust_add,sizeof(cust_add));  //taking the username from the client.
        flag = 0;
        while(read(fd_cust,&record,sizeof(record))){    //checking if the new username entered already exists in the file or not.
            if(strcmp(cust_add.username,record.username) == 0){ //if exists then send this flag to the client and ask for a new entry.
                flag = 1;
                write(sock, &flag,sizeof(int));
                break;
            }
        }
        lseek(fd_cust, 0, SEEK_SET);    //reset it to the start for a fresh search operation.
    }
    write(sock, &flag,sizeof(int)); //new username valid send a flag value 0 to client.
    close(fd_cust);

    char password[512];
    int Age;
    read(sock,password,sizeof(password));
    read(sock,&Age,sizeof(Age));

    strcpy(cust_add.password, password);
    long unsigned int acc_no =  (unsigned long)time(NULL);  //craeting a new and unique account number by getting the seconds from 1970 till today.
    cust_add.account_number=acc_no;
    cust_add.type = false;      //false signify that it is customer and not admin.
    cust_add.status = true;     // status true means active user.
    cust_add.age = Age;

    fd_cust=open("customer_db", O_WRONLY|O_APPEND);

    write(fd_cust,&cust_add,sizeof(cust_add));  // insert the value into the db file.
    close(fd_cust);

    struct Account acc_init;
    int fd2=open("account_db", O_WRONLY|O_APPEND);  //updating the value of the account for this customer.
    if(fd2!=-1){    //successfully opened.
		acc_init.account_number=acc_no;
        acc_init.balance=0.0;
        acc_init.status=true;
        write(fd2,&acc_init,sizeof(acc_init));  //update the account db file.
    }

    write(sock,&cust_add,sizeof(cust_add)); //send the details of the customer to the cleint to display message on the output screen.
    read(sock,&k,sizeof(k));        //get the input 1 from the client and reopen the adminMenu.
    if(k==1)
        adminMenu(sock,login);

}


int checkAcc(char name[]){
    struct Customer customer;

    int fd_cust = open("customer_db", O_RDONLY);
    
    while(read(fd_cust, &customer, sizeof(customer)))   //search through the customer db if the username is found or not and the user should be active.
    {
        if(!strcmp(customer.username, name) && customer.status == true)
        {
            close(fd_cust);
            return 1;
        }
    }
    close(fd_cust);
    return 0;    
}

void delAcc(int sock,struct Customer login){
    char name[512];
    read(sock,name,sizeof(name));   //get the username.
    char msg[100];
	int fd_cust, fd_acc, k;

    if(!checkAcc(name)) //boolean function that returns if the username is valid or not.
    {
        strcpy(msg, "Account not found");
        write(sock, msg, sizeof(msg));
    }
    else
    {   
		struct Customer cust;
		fd_cust = open("customer_db", O_RDWR);
		
		while(read(fd_cust, &cust, sizeof(cust))){  //search for the username.
            if(!strcmp(cust.username, name)){   //if found.
				lseek(fd_cust, -sizeof(cust), SEEK_CUR);    //go back 1 customer struct in the db file.
				cust.status = false;    //make it inactive customer.
				write(fd_cust,&cust,sizeof(cust));  //make the update in the db file of false or inactive customer.
				lseek(fd_cust, -sizeof(cust), SEEK_CUR);    //go back 1 customer struct in the db file.
				struct Account acc;
				fd_acc = open("account_db", O_RDWR);
				while(read(fd_acc,&acc,sizeof(acc))){   //search in the account db file for the customer.
					if(cust.account_number==acc.account_number){
						lseek(fd_acc, -sizeof(acc), SEEK_CUR);
						acc.status = false;
						write(fd_acc,&acc,sizeof(acc)); //update it.
						break;
					}
				}
				close(fd_acc);
				break;
				
				
			}			
		}
		close(fd_cust);
		
		strcpy(msg, "Account deleted");
        write(sock, msg, sizeof(msg));  //send message to the client.
    }
    read(sock,&k,sizeof(k));
    if(k==1)
        adminMenu(sock,login);
}



void search(int sock,struct Customer login){
    char name[512];
    read(sock,name,sizeof(name));   //recevei the username 
    char msg[100];
    int fd_acc, k;

    if(!checkAcc(name)) //boolean function that checks if the received username is valid and active or not.
    {
        strcpy(msg, "Account not found!");
        write(sock, msg, sizeof(msg));
    }
    else
    {   
        strcpy(msg, "");    //valid username found.
        write(sock, msg, sizeof(msg)); //sending an empty message.
        int fd_cust=open("customer_db", O_RDONLY);
        struct  Customer customer;
        
        while(read(fd_cust, &customer, sizeof(customer))){  //linear search to find the customer with the username.
            if(!strcmp(customer.username, name) && customer.status == true){
                write(sock,&customer, sizeof(customer));    //send the username to the client to display it.
                fd_acc=open("account_db",O_RDONLY);
                struct Account acc;
                while(read(fd_acc,&acc,sizeof(acc))){       //getting the account details by searching via the accoint number.
                    if(customer.account_number==acc.account_number){
                        write(sock,&acc,sizeof(acc));
                        break;
                    }
                }
                close(fd_acc);
                break;             
            }
        }
        close(fd_cust);
    }
    read(sock,&k,sizeof(k));
    if(k==1)
        adminMenu(sock,login);
}


void adminMenu(int sock,struct Customer login){     //called from the authorize function.
    int choice;
    int n;
    read(sock,&choice,sizeof(choice));
	switch(choice){
        case 1: 
				addAcc(sock,login);
				break;
        case 2: delAcc(sock,login); 
                break;
        case 3: search(sock,login);
                break;
        default: 
				break; 
    }
}


void deposit(int sock, struct Customer cust){
    double amount;
    double prev_bal,new_bal;
    int k,u;
    read(sock, &amount, sizeof(amount)); //get the amount.

    struct Account account;
    struct Transaction transaction;

    struct flock lock;  //apply write lock on the account and the transaction file. @ the start of the files.
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    time_t cur_time;
    struct tm *local;
    cur_time=time(NULL);

    local=localtime(&cur_time);
    
    int fd_acc=open("account_db",O_RDWR);
    int fd_trans=open("transaction_db",O_RDWR);
    int fcn_acc=fcntl(fd_acc, F_SETLKW, &lock); //apply the lock on accnt db
    int fcn_trans=fcntl(fd_trans, F_SETLKW, &lock);  //apply the lock on trans db
    if(fcn_acc<0||fcn_trans<0){perror("FCNTL::");}

    while(read(fd_acc,&account,sizeof(account))){   //linear search to get the account.
        if(cust.account_number==account.account_number)
        { 
            prev_bal=account.balance;
            write(sock,&prev_bal,sizeof(prev_bal)); //send the prev balance
            new_bal=prev_bal+amount;
            write(sock,&new_bal,sizeof(new_bal));   //send the new balacne.
            account.balance=new_bal;
            lseek(fd_acc,-sizeof(account),SEEK_CUR);    //go back 1 struct to the required one.
            write(fd_acc,&account,sizeof(account)); //update it.

            transaction.balance=new_bal;
            transaction.amount=amount;
            transaction.type=false;
            transaction.account_number=cust.account_number;
            strcpy(transaction.date,asctime(local));
            write(fd_trans,&transaction,sizeof(transaction)); //make a new entry in the transaction db file.
        }
    }
    read(sock,&u,sizeof(u));    //if 0 then unlock.
    if(u==0)
    {
        lock.l_type = F_UNLCK;
        int fl=fcntl(fd_acc, F_SETLKW, &lock);
        int fl2=fcntl(fd_trans, F_SETLKW, &lock);
        if(fl<0 || fl2<0){perror("ERROR:");}

        close(fd_trans);
        close(fd_acc);
    }
    read(sock,&k,sizeof(k));
    if(k==1)
        userMenu(sock,cust);
}

void withdraw(int sock, struct Customer cust){  //call from the user menu.
	double amount;
    double prev_bal,new_bal;
    int k,u;
    read(sock, &amount, sizeof(amount)); //get the amount value.

    struct Account account;
    struct Transaction transaction;

    struct flock lock;  //apply the lock on the account and the transaction db.
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    time_t cur_time;
    struct tm *local;
    cur_time=time(NULL);

    local=localtime(&cur_time);
    
    int fd_acc=open("account_db",O_RDWR);
    int fd_trans=open("transaction_db",O_RDWR);
    int fcn_acc=fcntl(fd_acc, F_SETLKW, &lock);  //lock on accnt db
    int fcn_trans=fcntl(fd_trans, F_SETLKW, &lock);  //lock on trans db
    if(fcn_acc == -1||fcn_trans == -1){perror("FCNTL::");}

    while(read(fd_acc,&account,sizeof(account))){   //linear saerch on the accnt db file
        if(cust.account_number==account.account_number)
        { 
            prev_bal=account.balance;
            write(sock,&prev_bal,sizeof(prev_bal)); //send the prev balance.
            if(prev_bal>=amount){   //if valid case...
                new_bal=prev_bal-amount;
                write(sock,&new_bal,sizeof(new_bal)); //send the  new  balance.
                account.balance=new_bal;
                lseek(fd_acc,-sizeof(account),SEEK_CUR);
                write(fd_acc,&account,sizeof(account)); //update the struct in the db.

                transaction.balance=new_bal;
                transaction.amount=amount;
                transaction.type=true;
                transaction.account_number=cust.account_number;
                strcpy(transaction.date,asctime(local));
                write(fd_trans,&transaction,sizeof(transaction));   //craete a new entry in the trans db.
            }
            
        }
    }
    read(sock,&u,sizeof(u)); //get 0 to unlock.
    if(u==0)
    {
        lock.l_type = F_UNLCK;
        int fl=fcntl(fd_acc, F_SETLK, &lock);
        int fl2=fcntl(fd_trans, F_SETLK, &lock);
        if(fl<0 || fl2<0){perror("ERROR:");}
        close(fd_trans);
        close(fd_acc);
    }
    read(sock,&k,sizeof(k));
    if(k==1)
        userMenu(sock,cust);
}

void passBook(int sock,struct Customer login){

    int fd_cust=open("customer_db", O_RDONLY);
    int fd_trans;
    int k;
    struct  Customer customer;
    
    while(read(fd_cust, &customer, sizeof(customer))){  //linear search in the cust db file for the customer details.
        if(!strcmp(customer.username, login.username)){
            fd_trans=open("transaction_db",O_RDONLY);
            struct Transaction trans;
            while(read(fd_trans,&trans,sizeof(trans))){ //now linear search in the transaction db file for the entries corresponding to the given cust account number.
                if(customer.account_number==trans.account_number){
                    write(sock,&trans,sizeof(trans));   //send the found details to the client to be printed.
                    break;
                }
            }
            close(fd_trans);
            break;             
        }
    }
    close(fd_cust);
    read(sock,&k,sizeof(k));
    if(k==1)
        userMenu(sock,login);
}


void viewDetails(int sock,struct Customer cust){
    int fd_acc=open("account_db",O_RDONLY);
    int fd_trans=open("transaction_db",O_RDONLY);
    int fd_cust=open("customer_db",O_RDONLY);
    int k;
    struct Account account;
    struct Customer customer;


    while(read(fd_acc,&account,sizeof(account))){   //here the details are sent from the userMenu itself.
        if(cust.account_number==account.account_number)
        {
            write(sock,&account,sizeof(account));   //find the account number in accnt db and sent it.
            break;
        }
    }
    while(read(fd_cust,&customer,sizeof(customer)))  //here the details are sent from the userMenu itself.
    {
        if(!strcmp(cust.username,customer.username))
        {
            write(sock,&customer,sizeof(customer)); //find the cust number in cust db and sent it.
            break;
        }
    }
    read(sock,&k,sizeof(k));
    if(k==1)
        userMenu(sock,cust);
}

void changePwd(int sock,struct Customer cust){

    char pass[512]="";          //here the cust contains the details of the customer whose password we have to change and it is sent from the USerMenu.
    char new_pass[512]="";
    read(sock,&pass,sizeof(pass)); //get the current password.
    int n,k;
    

    if(!strcmp(pass,cust.password))
    {
        n=1;
        write(sock,&n,sizeof(n));
        read(sock,&new_pass,sizeof(new_pass)); //get the new password.
        struct Customer customer;
        int fd_cust=open("customer_db",O_RDWR);
        while(read(fd_cust,&customer,sizeof(customer))){ //linear search in the cust db to get the cust details.
            if(!strcmp(customer.username,cust.username))
            {
                lseek(fd_cust,-sizeof(customer),SEEK_CUR); //go 1 struct back
                strcpy(customer.password,new_pass);     //update the struct customer in this code.
                write(fd_cust,&customer,sizeof(customer)); //update the struct in the db file.
                break;
            }
        }
    }
    else{
     write(sock,&n,sizeof(n));
 	}
 	read(sock,&k,sizeof(k));
	if(k==1)
	    userMenu(sock,cust);
}

void userMenu(int sock,struct Customer login){
    int choice;
    int n;
    long int acc_no;
	struct Customer cust;
    struct Account acc;
    int fd_cust=open("customer_db",O_RDONLY);
    while(read(fd_cust,&cust,sizeof(cust))){
        if(!strcmp(login.username,cust.username))
        {
			write(fd_cust,&cust,sizeof(cust));  // write the customer details here.
            break;
        }
    }
   
	int fd_trans=open("transaction_db",O_RDONLY);

    read(sock,&choice,sizeof(choice));  //get choice.
    switch(choice)
    {
        case 1: 
            deposit(sock,cust);
            break;
        case 2: 
            withdraw(sock,cust);
            break;
        case 3: 
            viewDetails(sock,cust); 
            break;
        case 4: 
            changePwd(sock,cust);
            break;
        case 5: 
            passBook(sock,cust);
            break;
        default:
            break;
    }
}

void authorize(int sock){
    struct Customer login;
    struct Customer record;
    read(sock, &login, sizeof(login));             //login structure stores the username and the password provided by the client through the terminal.
	int fd = open("customer_db", O_RDONLY);
    char msg[512]="";
    while(read(fd, &record, sizeof(record))){   //going through the records in the customer db.
        if(strcmp(record.username, login.username) == 0 && 
            strcmp(record.password, login.password) == 0 && record.status==true){   //if match found and active user.
            strcpy(msg, "Validation successful");
			write(sock, msg, sizeof(msg));

            if(record.type==true){
                adminMenu(sock,login);
                return;
            }
            else{
                userMenu(sock,login);
                return;
            }
        }
    }
    strcpy(msg, "Unauthorized User. Exiting...\n");
    write(sock, msg, sizeof(msg));
}


int main(){
    struct sockaddr_in server, client;
    int sock, size_client, cli;
    sock = socket(AF_INET,SOCK_STREAM,0);   //server structure creation with attributes.
    connect_admin();                        // creates the db files to store the data in the folder and create the admin.
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(46316);
    bind(sock, (void *)(&server), sizeof(server));
    listen(sock,10);

    while(1){
        size_client = sizeof(client);
        cli = accept(sock, (void *)(&client),&size_client);
        if(fork() == 0){
            authorize(cli);
        }else{
            close(cli);
        }
    } 
    return 0;
}
