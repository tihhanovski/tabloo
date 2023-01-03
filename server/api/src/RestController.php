<?php

/*
    endpoint    method  description         Auth
    qna
                GET     list of quizes      A
                POST    insert new          A
                PUT     update              A
    qna/<id>
                DEL     delete              A

    
    quiz/<link>
                GET     one quiz to answer  U
    
    answer/<link>
                POST    post answer         U
*/

class RestController {

    private $endpoints;

    function __construct() {
        $this->endpoints = [
            "authorities" => "AuthoritiesEndpoint",
            "areas" => "AreasEndpoint",
            "stops" => "StopsEndpoint",
            "moduletypes" => "MyModuletypesEndpoint",
            "modulemessage" => "MyModuleMessageEndpoint",
            "modules" => "MyModulesEndpoint",
            "mymodulesoutput" => "MyModulesOutputEndpoint",
        ];
    }

    private function getEndpoint(string $name): RestEndpoint {
        // app()->debug("ep: $name");
        if(!isset($this->endpoints[$name]))
            throw new NotFoundException("Endpoint \"$name\" not defined");
        $className = $this->endpoints[$name];
        $fn = __DIR__ . '/endpoints/' . $className . '.php';
        if(!file_exists($fn))
            throw new NotFoundException("Endpoint class \"$className\" not found");
        require_once $fn;
        $ret = new $className();
        return $ret;
    }

    private function outputCorrHeaders() {
        // output CORS headers
        header("access-control-allow-origin: *");
        header("access-control-allow-headers: Content-Type, origin, authorization, accept, client-security-token, host, date, cookie, cookie2, authtoken, x-debug, x-powered-by");
        header("access-control-allow-methods: " . ALLOWED_HTTP_METHODS);
    }

    public function run() {
        $method = app()->getRequestMethod();
        $this->outputCorrHeaders();
        if($method != 'options')
        {
            try {
                $allowedMethods = explode(",", strtolower(ALLOWED_HTTP_METHODS));
                if(!in_array($method, $allowedMethods))
                    throw new MethodNotAllowedException("Method $method is not allowed");


                $path = explode("/", app()->request("path"), 2);
                $ep = $path[0];
                $args = isset($path[1]) ? $path[1] : "";
                $endpoint = $this->getEndpoint($ep);

                if(!method_exists($endpoint, $method))
                    throw new MethodNotAllowedException("Method $method is not allowed for this endpoint");

                $ret = [
                    "status" => "ok",
                    "data" => $endpoint->$method($args),
                ];
    
            } catch (RestException $re) {
                //Catch our exceptions and provide feedback with meaningfull HTTP response code
                http_response_code($re->getHttpCode());
                $ret = [
                    "status" => "error",
                    "exception" => get_class($re),
                    "message" => $re->getMessage(),
                ];
            } catch (Throwable $e) {
                // catch anything we did not catch before and report it as internal error
                http_response_code(500);
                $ret =[
                        "status" => "error",
                        "exception" => get_class($e),
                        "message" => $e->getMessage(),
                ];
            }


            header("Content-Type: application/json; charset=UTF-8");
            $ret["debug"] = app()->getDebugLog();
            echo json_encode($ret);
        }
    }
}
