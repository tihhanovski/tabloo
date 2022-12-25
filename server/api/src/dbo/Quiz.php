<?php

    class Quiz {

        public $id, $title, $statusId, $userId, $permalink, $data;

        public function init(mixed $data, int $userId) {
            $this->data = $data;
            $this->userId = $userId;
            $this->statusId = 1;

            $d = json_decode($this->data);
            $this->title = $d->title;
        }

        public function markDeleted() {
            $this->statusId = 3;    //TODO
        }

        public static function fromPost(mixed $data, int $userId) {
            $inst = new self();
            $inst->init($data, $userId);
            return $inst;
        }

        public function checkCanBeEdited(int $userId): bool {
            if($this->userId != $userId)
                throw new UnauthorizedException("You cant edit this quiz");
            if($this->statusId != 1)
                throw new ForbiddenException("Cant modify published or deleted quiz");
            return true;
        }

        public function checkCanBePublished(int $userId): bool {
            if($this->userId != $userId)
                throw new UnauthorizedException("You cant edit this quiz");
            if($this->statusId != 1)
                throw new ForbiddenException("Cant modify published or deleted quiz");
            $this->validate();

            $qd = json_decode($this->data);
            app()->debug($qd);

            //checks before publishing

            if($qd->title == "")
                throw new ValidationException("No title");
            if(!($qd->questions && is_array($qd->questions) && count($qd->questions)) )
                throw new ValidationException("Quiz must have at least one question to be published");
            app()->debug("questions " . count($qd->questions));
            foreach($qd->questions as $qn) {
                if($qn->question == "")
                    throw new ValidationException("No question text");
                if(!($qn->answers && is_array($qn->answers) && count($qn->answers)))
                    throw new ValidationException("Every question must have at least one answer");
                if(!$qn->type)
                    throw new ValidationException("Question has no type");
                $correctCount = 0;
                foreach($qn->answers as $an) {
                    if($an->answer == "")
                        throw new ValidationException("No answer text");
                    if($an->correct)
                        $correctCount++;
                }

                switch($qn->type) {
                    case "single": {
                        if(count($qn->answers) < 2)
                            throw new ValidationException("Question with single correct answer should have at least two answers");
                        if($correctCount != 1)
                            throw new ValidationException("Question with single correct answer should have exactly one correct answer");
                        break;
                    }
                    case "multiple": {
                        break;
                    }
                    default: 
                        throw new ValidationException("Invalid question type");
                }
            }

            return true;
        }

        public function checkCanBeDeleted(int $userId): bool {
            if($this->userId != $userId)
                throw new UnauthorizedException("You cant delete this quiz");
            if($this->statusId == 3)
                throw new ForbiddenException("Cant delete deleted quiz");
            return true;
        }

        public function validate() {
            if(!$this->userId)
                throw new ValidationException("No user id");
            if(!$this->data)
                throw new ValidationException("No data");
            if(!$this->title)
                throw new ValidationException("No title");
        }

    }