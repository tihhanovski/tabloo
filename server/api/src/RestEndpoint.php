<?php

use Kreait\Firebase;
use Firebase\Auth\Token\Exception\InvalidToken;
use Kreait\Firebase\Factory;

class RestEndpoint {
    function process(string $path) {
        return ([
            "status" => "ok",
            "class" => get_class($this),
            "path" => $path
        ]);
    }
}

class AuthenticatedRestEndpoint extends RestEndpoint {

    protected /*User*/ $user;

    function __construct() {
        $this->authenticate();
    }
    
    function getAuthorizationHeader(): ?string {
        //see https://stackoverflow.com/questions/40582161/how-to-properly-use-bearer-tokens
        $headers = null;
        if (isset($_SERVER['Authorization'])) {
            $headers = trim($_SERVER["Authorization"]);
        }
        else if (isset($_SERVER['HTTP_AUTHORIZATION'])) { //Nginx or fast CGI
            $headers = trim($_SERVER["HTTP_AUTHORIZATION"]);
        } elseif (function_exists('getallheaders')) {
            $requestHeaders = getallheaders();
            // Server-side fix for bug in old Android versions (a nice side-effect of this fix means we don't care about capitalization for Authorization)
            $requestHeaders = array_combine(array_map('ucwords', array_keys($requestHeaders)), array_values($requestHeaders));
            //print_r($requestHeaders);
            if (isset($requestHeaders['Authorization'])) {
                $headers = trim($requestHeaders['Authorization']);
            }
        }

        return $headers;
    }

    // get access token from header
    // https://stackoverflow.com/questions/40582161/how-to-properly-use-bearer-tokens
    function getBearerToken(): ?string {
        $headers = $this->getAuthorizationHeader();
        if (!empty($headers)) {
            if (preg_match('/Bearer\s(\S+)/', $headers, $matches)) {
                return $matches[1];
            }
        }
        throw new UnauthorizedException("Authentication data is not provided");
    }

    function authenticate() {
    }

    function process(string $path) {
        $this->authenticate();
        return parent::process($path);
    }

}

class SimpleAuthenticatedRestEndpoint extends AuthenticatedRestEndpoint {
    function __construct() {
        $this->authenticate();
    }
    
    protected /*User*/ $user;

    function authenticate() {

    }
}

class ModuleOwnerAuthenticatedRestEndpoint extends AuthenticatedRestEndpoint {
    function __construct() {
        $this->authenticate();
    }
    
    protected /*User*/ $user;

    function authenticate() {
        $token = $this->getBearerToken();
        // app()->debug("auth $token");

        $sel = app()->db()->prepare("select id, name from moduleowners where apikey = :token and active = 1");
        $sel->execute([":token" => $token]);
        $this->user = $sel->fetch(PDO::FETCH_OBJ);
        if(!$this->user)
            throw new ForbiddenException("Unauthenticated");
        // app()->debug($this->user);
        return $this->user;
    }
}

/*
class FirebaseAuthenticatedRestEndpoint extends AuthenticatedRestEndpoint {

    function authenticate() {
        $idTokenString = $this->getBearerToken();
        if(!$idTokenString)
            throw new UnauthorizedException("Authentication data is not provided");
        try {
            // TODO use cache to speed things up here
            $factory = (new Factory)->withServiceAccount(FIREBASE_KEY);
            $auth = $factory->createAuth();            
            $verifiedIdToken = $auth->verifyIdToken($idTokenString);

            $uid = $verifiedIdToken->claims()->get('sub');
            $user = $auth->getUser($uid);
            $this->user = new User($uid, $user->email);

        } catch (Kreait\Firebase\Exception\Auth\FailedToVerifyToken $e) {
            throw new ForbiddenException($e->getMessage());
        }
    }

}
*/