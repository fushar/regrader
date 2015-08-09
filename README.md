Regrader 2
==========

An ICPC-style programming contest system that works.

***

> (Maintainer's note: in the previous version, Regrader's dependencies were committed to the repository. In this version, we start using dependency managers. To reduce the repository size, we removed the committed dependencies from git history. Therefore, unfortunately we had to **rewrite the entire git history**. If you are developing Regrader, please rebase your work accordingly. The previous version is still hosted on [https://github.com/fushar/regrader-v1](https://github.com/fushar/regrader-v1) as a backup.)

***

Built on top of:

- [CodeIgniter](https://ellislab.com/codeigniter)
- [Bootstrap](http://getbootstrap.com/)
- [Moe Contest Environment](http://www.ucw.cz/moe/)

Testimonial
-----------

*I needed a judging system for programming contest and spent days and nights on looking for one. Then I found Regrader - one of the best programming contest management systems on the internet. It's an open source project and extremely easy to install!*

*What I loved about the system is its flexibility and user-friendliness. With just a few clicks you are able to make a fully custom design of your Regrader page, suiting your needs. Moreover the system itself is stable and reliable.*

*Do what I did - try Regrader, you won't regret it!*

\- Vasil Sarafov, Bulgaria

Installation Guide
------------------------

#### 1. Install dependencies

Regrader requires these packages. Please install them.

- PHP >= 5.4
- MySQL

#### 2. Download Regrader

Download a copy of the latest Regrader: https://github.com/fushar/regrader/releases/download/v2.1.0/regrader-v2.1.0.tar.gz

#### 3. Configure Regrader

Create an InnoDB database in your MySQL database server. Then, open the database configuration file **application/config/database.php** and add your hostname, username, password, and database name in this part of the file:

```php
$db['default'] = array(
    //
    'hostname' 		=> 'localhost',
    'username' 		=> '',
    'password' 		=> '',
    'database' 		=> '',
    //
);
```

Next, open the configuration file **application/config/config.php** and add a random string in this part of the file:

```php
$config['encryption_key'] = '';
```

It is advisable that the string consists of 32 random characters.

Finally, make sure your ``php.ini`` file has ``date.timezone`` option set.

#### 4. Install Regrader

Execute this command:

```
./install.sh
```

Then, open Regrader in your browser, and perform the installation steps as instructed. If at any step the system cannot install at some specified directories, please add write permission on them.

#### 5. Run the grader engine

Execute `run_grader.sh` script on your host:

```
./run_grader.sh
```

Configuring the System
----------------------

#### 1. Configure programming languages

Open **Manage**->**Languages**. There will be 4 programming languages set up by default: Pascal, Java, C++, and C. For each language you want to actually use, you have to at least edit its compile command as necessary depending where you install the compiler.

#### 2. Configure additional options

Open **Manage**->**Options** and configure additional properties of the system.

- **Website, Top, & Bottom Names**
To be shown in the header.
- **Left, Right 1, Right 2 Logo Filenames**
To be shown in the header. If set, the logo files must be uploaded in **Manage**->**Files**.
- **Items per Page**
The number of rows in a table. Used in the list of users, problems, submissions, etc.

Using the System
------------------

After installation, you can log in and add users, contests, problems, etc. The interface should be quite intuitive to use :)

Miscellaneous
-------------

#### Solution checker

You can set up a solution checker for your problems. This is useful for problems that can have multiple solutions. The template for a checker can be found here: https://github.com/fushar/regrader/blob/develop/examples/checker.cpp.

#### Internationalization

Regrader currently supports English and Indonesian. To change the language to Indonesian, open **application/config/config.php** and change the value of **$config['language']** into **'indonesian'**.

#### Institution logos

You can show institution logos beside the contestant names in the scoreboard. The scoreboard will look very nice. Just upload **X**.jpg in **Manage->Files** as the logo for institution **X**. Then, tick the option **Show Institution Logos** in the corresponding contest's setting.

#### Public files

You can use **Manage**->**Files** to upload public files, for example, for inserting images in problem statements.

#### Public raw scoreboard

Besides the usual public scoreboard, you can also generate public scoreboard that is separated from the system. This is useful for hiding the contest URL address from the public and in order to minimize traffic and external attack.

- Download the public raw scoreboard HTML file from the admin scoreboard menu.
- Copy the **assets/** directory.
- Publish both in a separate address.

Of course, to achieve a semi-live scoreboard, you have to do the above steps periodically, possibly using a script.

Got any weird errors?
---------------------

Here are common suggestions that should solve most possible errors:

- Make sure that you have set up compile commands in programming languages you want to use.
- Make sure that your server has execute permission on **moe/obj/box/box**.
- Make sure that your test case output files have '\n' as the line ending.
- Make sure that your test case files have extensions.
- Make sure that **run_grader.sh** is running.

License
-------

Regrader is licensed under MIT License.

Contributing
------------

Contributions are welcome! To set up Regrader for development, you need:

- [composer](https://getcomposer.org/)
- [bower](http://bower.io/)

After that, fork and clone Regrader. Then, run

```
./build.sh
```

You are then ready to develop.

To produce a tar.gz that is ready to deploy, run

```
./publish.sh
```

Mantainers
----------

Currently, Regrader is maintained by:

- Ashar Fuadi ([@fushar](https://github.com/fushar))
- Pusaka Kaleb Setyabudi ([@sokokaleb](https://github.com/sokokaleb))
- Rakina Zata Amni ([@rakina](https://github.com/rakina))
