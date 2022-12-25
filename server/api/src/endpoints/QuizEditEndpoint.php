<?php

/*
    endpoint    method  description         Auth
    qna
                GET     list of quizes      A
                POST    insert new          A
                PUT     update              A
    qna/<id>
                GET     selected quiz       A
                DEL     delete              A
*/

class QuizEditEndpoint extends AuthenticatedRestEndpoint {

    public function get(string $path) {
        if($path != ""){
            $id = (int)$path;
            $quiz = Quizes::get($id);
            $quiz->checkCanBeEdited($this->user->getId());
                return json_decode($quiz->data);
        }
        else
            return Quizes::getList($this->user->getId());
    }

    public function post(string $path) {
        $data = app()->getRequestPayload();
        $quiz = Quiz::fromPost($data, $this->user->getId());
        $quiz->validate();
        $inserted = Quizes::insert($quiz);
        return [
            "id" => $inserted->id,
            "record" => json_decode($inserted->data),
        ];
    }

    public function put(string $path) {
        $data = app()->getRequestPayload();
        $id = (int)$path;
        if(!$id)
            throw new ForbiddenException("Cant update this quiz");

        //TODO
        //load data from db
        $quiz = Quizes::get($id);
        //check if able to update
        // - not deleted
        // - not published
        // - user is the same
        $quiz->checkCanBeEdited($this->user->getId());
        $quiz->init($data, $this->user->getId());
        // validate object
        $quiz->validate();
        // update object
        Quizes::update($quiz);

        return [
            "record" => json_decode($quiz->data),
        ];
    }

    public function delete(string $path) {
        $data = app()->getRequestPayload();
        $id = (int)$path;
        if(!$id)
            throw new ForbiddenException("Provide quiz id to delete");
        //TODO
        //load data from db
        $quiz = Quizes::get($id);
        $quiz->checkCanBeDeleted($this->user->getId());
        Quizes::delete($id);

        return [
            "deletedId" => $id
        ];
    }
}
