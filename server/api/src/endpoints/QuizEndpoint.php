<?php

class QuizEndpoint extends RestEndpoint {
    
    public function get(string $path) {
        Quizes::checkPermalink($path);
        $quiz = Quizes::find($path);

        //build data with stripped answers
        $d = json_decode($quiz->data);
        $questions = [];
        foreach($d->questions as $q) {
            $answers = [];
            foreach($q->answers as $a) {
                $answers[] = $a->answer;
                // Another version
                // $answers[] = [
                //     "answer" => $a->answer
                // ];
            }
            $questions[] = [
                "question" => $q->question,
                "type" => $q->type,
                "answers" => $answers
            ];

        }
        $data = [
            "title" => $d->title,
            "questions" => $questions,
        ];

        return $data;
    }
}