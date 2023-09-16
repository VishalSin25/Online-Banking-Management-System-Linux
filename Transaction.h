/* Structure to store the Transaction for  Banking Management System.
Name: Subham Basu Roy Chowdhury
Roll: MT2022118
*/

struct Transaction{
        char date[128];
        long int account_number;
        bool type;              //true for {withdraw/debit} and false for {credit/deposit}.
        double amount;
        double balance;
};

//entries are like a queue, here we don't store data in a sequence. For searching we travese throught the file and match the account number
