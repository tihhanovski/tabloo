<?php

const SQL_QUIZES_LIST = "SELECT q.id, q.title, s.title as status, q.permalink
    FROM quizes q left join quizstatuses s on s.id = q.statusId
    WHERE q.userId = :userId and q.statusId in (1, 2)";
const SQL_INSERT_QUIZ = "INSERT INTO quizes (userId, title, statusId, permalink, data) 
    VALUES(:userId, :title, :statusId, :permalink, :data)";
const SQL_SELECT_QUIZ = "select id, userId, title, statusId, permalink, data 
    from quizes where id = :id";
const SQL_UPDATE_QUIZ = "update quizes set userId = :userId, title = :title, 
    statusId = :statusId, permalink = :permalink, data = :data 
    where id = :id";
const SQL_DELETE_QUIZ = "update quizes set statusId = 3 where id = :id";
const SQL_FIND_QUIZ = "select * from quizes 
    where permalink = :permalink and statusId = 2";

const SQL_PUBLISH_QUIZ = "update quizes set statusId = 3, permalink = :permalink 
    where statusId = 1 and userId = :userId and id = :id";


class Quizes {

    public static function checkPermalink(string $permalink) {
        if($permalink == "")
            throw new BadRequestException("Permalink not provided");
        if(strlen($permalink) > PERMALINK_MAX_LENGTH)
            throw new BadRequestException("Quiz permalink is too long");
    }

    public static function getList(int $userId) {
        $stmt = app()->db()->prepare(SQL_QUIZES_LIST);
        $stmt->execute([":userId" => $userId]);
        return $stmt->fetchAll(PDO::FETCH_ASSOC);
    }

    public static function get(int $id) : Quiz {
        $sel = app()->db()->prepare(SQL_SELECT_QUIZ);
        $sel->execute([":id" => $id]);
        $sel->setFetchMode(PDO::FETCH_CLASS, 'Quiz');
        $ret = $sel->fetch();
        if(!$ret)
            throw new NotFoundException("Quiz not found");
        return $ret;
    }

    public static function find(string $permalink) : Quiz {
        $sel = app()->db()->prepare(SQL_FIND_QUIZ);
        $sel->execute([":permalink" => $permalink]);
        $sel->setFetchMode(PDO::FETCH_CLASS, 'Quiz');
        $ret = $sel->fetch();
        if(!$ret)
            throw new NotFoundException("Quiz not found");
        return $ret;
    }

    public static function insert(Quiz $quiz) {
        $stmt = app()->db()->prepare(SQL_INSERT_QUIZ);
        $vars = get_object_vars($quiz);
        unset($vars["id"]);
        $stmt->execute($vars);
        $quiz->id = app()->db()->lastInsertId();

        $sel = app()->db()->prepare(SQL_SELECT_QUIZ);
        $sel->execute([":id" => $quiz->id]);
        $sel->setFetchMode(PDO::FETCH_CLASS, 'Quiz');
        return $sel->fetch();
    }

    public static function update(Quiz $quiz) {
        $stmt = app()->db()->prepare(SQL_UPDATE_QUIZ);
        $vars = get_object_vars($quiz);
        $stmt->execute($vars);
    }

    public static function publish(Quiz $quiz) {
        $lastException = "";
        for($x = 0; $x < MAX_PERMALINK_GENERATION_ATTEMPTS; $x++)
            try {
                $quiz->statusId = 2;    //TODO
                $quiz->permalink = substr(md5(microtime()),rand(0,26),6);
                self::update($quiz);
                return;
            } catch (PDOException $e) {
                $lastException = $e->getMessage();
            }
        throw new RestException("Unable to publish quiz. " . $lastException);
    }

    public static function delete(int $id) {
        $del = app()->db()->prepare(SQL_DELETE_QUIZ);
        $del->execute([":id" => $id]);
        app()->debug("delete result: " . $del->errorCode());
        // TODO check if delete successfull

    }

}