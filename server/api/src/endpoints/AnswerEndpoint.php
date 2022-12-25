<?php

class AnswerEndpoint extends RestEndpoint {

    public function post(string $path) {
        Quizes::checkPermalink($path);
        $quiz = Quizes::find($path);
        $pl = app()->getRequestPayload();

        if($pl == "")
            throw new BadRequestException("Answer not provided");
        try {
            $answer = json_decode($pl);
        }catch(Throwable $t) {
            throw new BadRequestException($t->getMessage());
        }

        $qdata = json_decode($quiz->data);

        $qCount = 0;
        $aCount = 0;

        $aa = [];
        foreach($answer as $a) {
            sort($a->answers);
            $aa[$a->question] = implode("\t", $a->answers);
        }

        foreach($qdata->questions as $q) {
            $qCount++;
            $correctAnswers = [];
            foreach($q->answers as $a)
                if($a->correct)
                    $correctAnswers[] = $a->answer;
            sort($correctAnswers);
            $sCorrectAnswers = implode("\t", $correctAnswers);

            if($sCorrectAnswers == $aa[$q->question])
                $aCount++;
        }


        return [
            "questions" => $qCount,
            "correct" => $aCount,
        ];
    }
}