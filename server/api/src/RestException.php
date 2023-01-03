<?php

// https://www.semrush.com/blog/http-status-codes/

class RestException extends Exception {
    protected $httpCode = 400;

    public function getHttpCode(): int {
        return $this->httpCode;
    }
}

class TODOException extends RestException {
    protected $httpCode = 404;
}

class BadRequestException extends RestException {
    protected $httpCode = 400;
}

class NotFoundException extends RestException {
    protected $httpCode = 404;
}

class MethodNotAllowedException extends RestException {
    protected $httpCode = 405;
}

class UnauthorizedException extends RestException {
    protected $httpCode = 401;
}

class ForbiddenException extends RestException {
    protected $httpCode = 403;
}

class ValidationException extends RestException {
    protected $httpCode = 400;
}