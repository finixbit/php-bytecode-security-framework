# List of vulnerabilties for `Vulnerable Web applications Test cases`
Before running any of the test cases below, you need to set the docker container by following the [Setup In README.md](README.md#setting-up)

## Testing with `DVWA`
`Damn Vulnerable Web Application (DVWA)`

### Testing
```sh
cd /app
git clone https://github.com/digininja/DVWA targets/DVWA
python3 detectors/code_injection.py
```

### Reports
```sh
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] TOTAL_VULNS = 21
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 1
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/view_help.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=14 ------ $id       = $_GET[ 'id' ];
[*] lineno=20 ------ eval( '?>' . file_get_contents( DVWA_WEB_PAGE_TO_ROOT . "vulnerabilities/{$id}/help/help.php" ) . '<?php ' );
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 2
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/view_source.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=13 ------ $security = $_GET[ 'security' ];
[*] lineno=60 ------ $source = @file_get_contents( DVWA_WEB_PAGE_TO_ROOT . "vulnerabilities/{$id}/source/{$security}.php" );
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 3
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/view_source_all.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=12 ------ $id = $_GET[ 'id' ];
[*] lineno=14 ------ $lowsrc = @file_get_contents("./{$id}/source/low.php");
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 4
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/brute/source/low.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=5 ------ $user = $_GET[ 'username' ];
[*] lineno=12 ------ $query  = "SELECT * FROM `users` WHERE user = '$user' AND password = '$pass';";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 5
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/captcha/source/low.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=51 ------ $pass_conf = $_POST[ 'password_conf' ];
[*] lineno=33 ------ <input type=\"hidden\" name=\"password_conf\" value=\"{$pass_conf}\" />
[*] lineno=32 ------ <input type=\"hidden\" name=\"password_new\" value=\"{$pass_new}\" />
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 6
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/captcha/source/low.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=51 ------ $pass_conf = $_POST[ 'password_conf' ];
[*] lineno=33 ------ <input type=\"hidden\" name=\"password_conf\" value=\"{$pass_conf}\" />
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 7
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/captcha/source/medium.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=52 ------ $pass_conf = $_POST[ 'password_conf' ];
[*] lineno=33 ------ <input type=\"hidden\" name=\"password_conf\" value=\"{$pass_conf}\" />
[*] lineno=32 ------ <input type=\"hidden\" name=\"password_new\" value=\"{$pass_new}\" />
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 8
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/captcha/source/medium.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=52 ------ $pass_conf = $_POST[ 'password_conf' ];
[*] lineno=33 ------ <input type=\"hidden\" name=\"password_conf\" value=\"{$pass_conf}\" />
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 9
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/csp/source/high.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=10 ------ " . $_POST['include'] . "
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 10
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/csp/source/impossible.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=11 ------ " . $_POST['include'] . "
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 11
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/csp/source/jsonp.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=5 ------ $callback = $_GET['callback'];
[*] lineno=12 ------ echo $callback . "(".json_encode($outp).")";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 12
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/csp/source/low.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=15 ------ <script src='" . $_POST['include'] . "'></script>
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 13
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/csp/source/medium.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=16 ------ " . $_POST['include'] . "
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 14
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/exec/source/low.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=5 ------ $target = $_REQUEST[ 'ip' ];
[*] lineno=10 ------ $cmd = shell_exec( 'ping  ' . $target );
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 15
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/open_redirect/source/high.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=5 ------ header ("location: " . $_GET['redirect']);
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 16
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/open_redirect/source/low.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=4 ------ header ("location: " . $_GET['redirect']);
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 17
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/open_redirect/source/medium.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=11 ------ header ("location: " . $_GET['redirect']);
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 18
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/sqli/source/low.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=5 ------ $id = $_REQUEST[ 'id' ];
[*] lineno=10 ------ $query  = "SELECT first_name, last_name FROM users WHERE user_id = '$id';";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 19
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/sqli_blind/source/high.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=5 ------ $id = $_COOKIE[ 'id' ];
[*] lineno=11 ------ $query  = "SELECT first_name, last_name FROM users WHERE user_id = '$id' LIMIT 1;";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 20
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/sqli_blind/source/low.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=5 ------ $id = $_GET[ 'id' ];
[*] lineno=11 ------ $query  = "SELECT first_name, last_name FROM users WHERE user_id = '$id';";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 21
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/DVWA/vulnerabilities/xss_r/source/low.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=8 ------ $html .= '<pre>Hello ' . $_GET[ 'name' ] . '</pre>';
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] TOTAL_VULNS = 21
[*]
```

### Fixes
```
N/A
```

---

## Testing with `OWASP-vuln-web-app`
`OWASP Vulnerable Web Application Project`

### Testing
```sh
cd /app
git clone https://github.com/OWASP/Vulnerable-Web-Application targets/Vulnerable-Web-Application
python3 detectors/code_injection.py
```

### Reports
```sh
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] TOTAL_VULNS = 10
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 1
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/Vulnerable-Web-Application/FileInclusion/pages/lvl1.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=25 ------ @include($_GET[ 'file' ]);
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 2
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/Vulnerable-Web-Application/FileInclusion/pages/lvl1.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=26 ------ echo"<div align='center'><b><h5>".$_GET[ 'file' ]."</h5></b></div> ";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 3
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/Vulnerable-Web-Application/FileUpload/fileupload1.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=28 ------ echo "File uploaded /uploads/".$_FILES["file"]["name"];
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 4
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/Vulnerable-Web-Application/FileUpload/fileupload2.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=37 ------ echo "File uploaded /uploads/".$_FILES["file"]["name"];
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 5
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/Vulnerable-Web-Application/FileUpload/fileupload3.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=39 ------ echo "File uploaded /uploads/".$_FILES["file"]["name"];
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 6
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/Vulnerable-Web-Application/SQL/sql1.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=39 ------ $firstname = $_POST["firstname"];
[*] lineno=40 ------ $sql = "SELECT lastname FROM users WHERE firstname='$firstname'";//String
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 7
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/Vulnerable-Web-Application/SQL/sql2.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=37 ------ $number = $_POST['number'];
[*] lineno=38 ------ $query = "SELECT bookname,authorname FROM books WHERE number = $number"; //Int
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 8
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/Vulnerable-Web-Application/SQL/sql3.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=38 ------ $number = $_POST['number'];
[*] lineno=39 ------ $query = "SELECT bookname,authorname FROM books WHERE number = '$number'"; //Is this same with the level 2?
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 9
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/Vulnerable-Web-Application/SQL/sql6.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=35 ------ $number = $_GET['number'];
[*] lineno=36 ------ $query = "SELECT bookname,authorname FROM books WHERE number = '$number'";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 10
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/Vulnerable-Web-Application/XSS/XSS_level1.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=22 ------ echo("Your name is ".$_GET["username"])?>
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] TOTAL_VULNS = 10
[*]
```

### Fixes
```
N/A
```

---

## Testing with `sqlinjection-training-app`
`Simple SQL Injection Training App`

### Testing
```sh
cd /app
git clone https://github.com/appsecco/sqlinjection-training-app targets/sqlinjection-training-app
python3 detectors/code_injection.py
```

### Reports
```sh
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] TOTAL_VULNS = 15
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 1
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/sqlinjection-training-app/www/blindsqli.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=29 ------ $user = $_GET["user"];
[*] lineno=30 ------ $q = "Select * from users where username = '".$user."'";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 2
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/sqlinjection-training-app/www/login1.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=64 ------ $username = ($_REQUEST['uid']);
[*] lineno=67 ------ $q = "SELECT * FROM users where username='".$username."' AND password = '".md5($pass)."'" ;
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 3
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/sqlinjection-training-app/www/login2.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=65 ------ $username = ($_REQUEST['uid']);
[*] lineno=68 ------ $q = "SELECT * FROM users where (username='".$username."') AND (password = '".md5($pass)."')" ;
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 4
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/sqlinjection-training-app/www/os_sqli.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=30 ------ $user = $_GET["user"];
[*] lineno=31 ------ $q = "Select * from users where username = '".$user."'";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 5
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/sqlinjection-training-app/www/register.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=93 ------ $username = $_REQUEST['uid'];
[*] lineno=98 ------ $q = "INSERT INTO users (username, password, fname, description) values ('".$username."','".md5($pass)."','".$fname."','".$descr."')" ;
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 6
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/sqlinjection-training-app/www/register.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=95 ------ $fname = $_REQUEST['name'];
[*] lineno=98 ------ $q = "INSERT INTO users (username, password, fname, description) values ('".$username."','".md5($pass)."','".$fname."','".$descr."')" ;
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 7
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = md5
[*] FILE = /app/targets/sqlinjection-training-app/www/register.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=95 ------ $fname = $_REQUEST['name'];
[*] lineno=98 ------ $q = "INSERT INTO users (username, password, fname, description) values ('".$username."','".md5($pass)."','".$fname."','".$descr."')" ;
[*] lineno=93 ------ $username = $_REQUEST['uid'];
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 8
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/sqlinjection-training-app/www/register.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=96 ------ $descr = $_REQUEST['descr'];
[*] lineno=98 ------ $q = "INSERT INTO users (username, password, fname, description) values ('".$username."','".md5($pass)."','".$fname."','".$descr."')" ;
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 9
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = md5
[*] FILE = /app/targets/sqlinjection-training-app/www/register.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=96 ------ $descr = $_REQUEST['descr'];
[*] lineno=95 ------ $fname = $_REQUEST['name'];
[*] lineno=98 ------ $q = "INSERT INTO users (username, password, fname, description) values ('".$username."','".md5($pass)."','".$fname."','".$descr."')" ;
[*] lineno=93 ------ $username = $_REQUEST['uid'];
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 10
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/sqlinjection-training-app/www/searchproducts.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=56 ------ $q = "Select * from products where product_name like '".$_POST["searchitem"]."%'";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 11
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/sqlinjection-training-app/www/secondorder_home.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=117 ------ $username = $_REQUEST['uid'];
[*] lineno=122 ------ $q = "INSERT INTO users (username, password, fname, description) values ('".$username."','".md5($pass)."','".$fname."','".$descr."')" ;
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 12
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/sqlinjection-training-app/www/secondorder_home.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=119 ------ $fname = $_REQUEST['name'];
[*] lineno=122 ------ $q = "INSERT INTO users (username, password, fname, description) values ('".$username."','".md5($pass)."','".$fname."','".$descr."')" ;
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 13
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = md5
[*] FILE = /app/targets/sqlinjection-training-app/www/secondorder_home.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=119 ------ $fname = $_REQUEST['name'];
[*] lineno=122 ------ $q = "INSERT INTO users (username, password, fname, description) values ('".$username."','".md5($pass)."','".$fname."','".$descr."')" ;
[*] lineno=117 ------ $username = $_REQUEST['uid'];
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 14
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/sqlinjection-training-app/www/secondorder_home.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=120 ------ $descr = $_REQUEST['descr'];
[*] lineno=122 ------ $q = "INSERT INTO users (username, password, fname, description) values ('".$username."','".md5($pass)."','".$fname."','".$descr."')" ;
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 15
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = md5
[*] FILE = /app/targets/sqlinjection-training-app/www/secondorder_home.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=120 ------ $descr = $_REQUEST['descr'];
[*] lineno=119 ------ $fname = $_REQUEST['name'];
[*] lineno=122 ------ $q = "INSERT INTO users (username, password, fname, description) values ('".$username."','".md5($pass)."','".$fname."','".$descr."')" ;
[*] lineno=117 ------ $username = $_REQUEST['uid'];
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] TOTAL_VULNS = 15
[*]

```

### Fixes
```
N/A
```

---

## Testing with `OSTE-Vulnerable-Web-Application`
`Vulnerable Web application made with PHP/SQL designed to help new web testers gain some experience and test DAST tools for identifying web vulnerabilities.`

### Testing
```sh
cd /app
git clone https://github.com/OSTEsayed/OSTE-Vulnerable-Web-Application targets/OSTE-Vulnerable-Web-Application
python3 detectors/code_injection.py
```

### Reports
```sh
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] TOTAL_VULNS = 6
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 1
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/OSTE-Vulnerable-Web-Application/SQL/page1.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=210 ------ $passe=$_POST["pswd"];
[*] lineno=212 ------ $sql = "INSERT INTO user (name,password) VALUES ('$namee','$passe')";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 2
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/OSTE-Vulnerable-Web-Application/SQL/page1.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=209 ------ $namee=$_POST["username"];
[*] lineno=212 ------ $sql = "INSERT INTO user (name,password) VALUES ('$namee','$passe')";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 3
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/OSTE-Vulnerable-Web-Application/SQL/page4.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=212 ------ $namee=$_POST["username"];
[*] lineno=215 ------ $sql3 = "SELECT * FROM books WHERE author='$namee'";//String
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 4
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/OSTE-Vulnerable-Web-Application/SQL/page5.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=212 ------ $namee=$_POST["username"];
[*] lineno=215 ------ $sql3 = "SELECT * FROM sport WHERE id='$namee'";//String
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 5
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/OSTE-Vulnerable-Web-Application/SQL/page6.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=212 ------ $namee=$_GET["username"];
[*] lineno=215 ------ $sql3 = "SELECT * FROM sport WHERE id='$namee'";//String
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 6
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/OSTE-Vulnerable-Web-Application/XSS/page1.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=197 ------ echo$_POST['username'];
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] TOTAL_VULNS = 6
[*]
```

### Fixes
```
N/A
```

---

## Testing with `InsecureTrust_Bank`
`"InsecureTrust_Bank: Educational repo demonstrating web app vulnerabilities like SQL injection & XSS for security awareness. Use responsibly.`

### Testing
```sh
cd /app
git clone https://github.com/Hritikpatel/InsecureTrust_Bank targets/InsecureTrust_Bank
python3 detectors/code_injection.py
```

### Reports
```sh
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] TOTAL_VULNS = 5
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 1
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/InsecureTrust_Bank/assets/php_process/login_process.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=9 ------ $entered_password = $_POST["password"];
[*] lineno=27 ------ $sql = "SELECT * FROM logininfo WHERE username = '$entered_username' AND password = '$entered_password'";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 2
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/InsecureTrust_Bank/assets/php_process/login_process.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=8 ------ $entered_username = $_POST["username"];
[*] lineno=27 ------ $sql = "SELECT * FROM logininfo WHERE username = '$entered_username' AND password = '$entered_password'";
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 3
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/InsecureTrust_Bank/public/faq.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=50 ------ $searchQuery = $_GET[ 'search' ];
[*] lineno=53 ------ $out = '<pre> Looking for ' . $searchQuery . '</pre>';
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 4
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/InsecureTrust_Bank/public/faq.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=50 ------ $searchQuery = $_GET[ 'search' ];
[*] lineno=53 ------ $out = '<pre> Looking for ' . $searchQuery . '</pre>';
[*] lineno=54 ------ echo $out;
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] INDEX = 5
[*] IMPACT = HIGH
[*] VULNERABILTY TYPE = code_injection
[*] VISITED FUNCTION CALLS = 
[*] FILE = /app/targets/InsecureTrust_Bank/toolPvt/supportApi.php::main
[*] 
[*] --- SOURCE PATH TO VULNERABILTY ---
[*] lineno=69 ------ $number = $_GET["number"];
[*] lineno=72 ------ $stmt = $pdo->prepare("SELECT * FROM account WHERE Phone == $number");
[*]
[*]
[*] ----------------------------------------------------------------------------------------------------
[*] ----------------------------------------------------------------------------------------------------
[*] TOTAL_VULNS = 5
[*]
```

### Fixes
```
N/A
```

---
