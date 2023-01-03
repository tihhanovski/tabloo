<?php

const SQL_GET_USER_ID = "select id from users where email = :email";
const SQL_INSERT_USER = "insert into users(uid, email)values(:uid, :email)";

class User {
    private $id;
    public $uid, $email;   //TODO

    function __construct($uid, $email) {
        $this->uid = $uid;
        $this->email = $email;
    }

    function getId(): int {
        if(!isset($this->id)) {
            //user id will be retrieved only once
            $stmt = app()->db()->prepare(SQL_GET_USER_ID);
            $stmt->execute([":email" => $this->email]);
            $u = $stmt->fetch(PDO::FETCH_ASSOC);
            if($u && isset($u["id"]))
                $this->id = $u["id"];
            else {
                //no user in database, add it:
                $stmt = app()->db()->prepare(SQL_INSERT_USER);
                $stmt->execute([
                    ":uid" => $this->uid, 
                    ":email" => $this->email
                ]);

                //this might not work with other databases
                $this->id = app()->db()->lastInsertId();
            }    
        }
        return $this->id;
    }
}