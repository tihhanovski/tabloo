<?php

class QuizPublishEndpoint extends AuthenticatedRestEndpoint {

    public function get(string $path) {
        $id = (int)$path;
        if($id) {            
            $quiz = Quizes::get($id);
            $quiz->checkCanBePublished($this->user->getId());
            Quizes::publish($quiz);
            return [
                "status" => "ok",
            ];
        }
        else
            throw new BadRequestException("Quiz id should be provided");
    }
}