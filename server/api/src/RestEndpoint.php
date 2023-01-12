<?php

/**
 * Rest endpoint base class
 */
class RestEndpoint {

    /**
     * @deprecated
     */
    function process(string $path) {
        return ([
            "status" => "ok",
            "class" => get_class($this),
            "path" => $path
        ]);
    }
}

/**
 * Endpoint that needs authentication
 */
class AuthenticatedRestEndpoint extends RestEndpoint {

    // Authenticated user data
    protected /*User*/ $user;

    /**
     * Constructor
     * authenticate during creating instance
     */
    function __construct() {
        $this->authenticate();
    }
    
    /**
     * Extracts auth header from data provided by web server (apache, nginx(?))
     * Tested on apache on typical LAMP install
     */
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

    /**
     * Override in descendants
     */
    function authenticate() {
    }

    /**
     * @deprecated
     */
    function process(string $path) {
        $this->authenticate();
        return parent::process($path);
    }

}

/**
 * Module owner endpoint
 * Authenticated using module owner's api key
 */
class ModuleOwnerAuthenticatedRestEndpoint extends AuthenticatedRestEndpoint {

    /**
     * Authenticate user using API key stored in database (moduleowners.apikey)
     */
    function authenticate() {
        // Get auth token
        $token = $this->getBearerToken();

        if(!$token)
            throw new ForbiddenException("No token provided");

        // Find id and name for active user with given key
        $sel = app()->db()->prepare("select id, name from moduleowners where apikey = :token and active = 1");
        $sel->execute([":token" => $token]);
        $this->user = $sel->fetch(PDO::FETCH_OBJ);
        if(!$this->user)
            throw new ForbiddenException("Unauthenticated");

        return $this->user;
    }
}
