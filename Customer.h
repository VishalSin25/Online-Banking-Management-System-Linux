/* Structure to store the Customer for  Banking Management System.
Name: Subham Basu Roy Chowdhury
Roll: MT2022118
*/

struct Customer{
        char username[512];	//Primary Account holder
        //char username2[512];	//Secondary Account Holder, for Normal User set = "".
        char password[512];
        long int account_number;
        int age;
        int type;		// 0-> admin 1-> Normal User 2-> Joint User.
        bool status;		// 0-> INactive 1-> Active
};
