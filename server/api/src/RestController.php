<?php
/**
 * Tabloo - open source bus stop information display
 * 
 * Rest controller class
 * 
 * @author ilja.tihhanovski@gmail.com
 * 
 */

/**
 * Main REST controller
 */
class RestController {

    // List of enabled endpoints
    private $endpoints;

    function __construct() {
        // Initialize list of enabled endpoints
        // Key - endpoint URI
        // Value - endpoint class name. Class should be situated in endpoints/<className>.php
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

    /**
     * Return class for endpoint requested
     * @param name endpoint name
     * @return object for endpoint
     * @throws RestException if something went wrong
     */
    private function getEndpoint(string $name): RestEndpoint {

        // check if endpoint enabled
        if(!isset($this->endpoints[$name]))
            throw new NotFoundException("Endpoint \"$name\" not defined");
        $className = $this->endpoints[$name];
        $fn = __DIR__ . '/endpoints/' . $className . '.php';

        // check if endpoint file exist
        if(!file_exists($fn))
            throw new NotFoundException("Endpoint class \"$className\" not found");

        // include endpoint class definition, instantiate class and return instance
        require_once $fn;
        $ret = new $className();
        return $ret;
    }

    /**
     * Output CORS headers to enable web access
     */
    private function outputCorrHeaders() {
        // output CORS headers
        header("access-control-allow-origin: *");
        header("access-control-allow-headers: Content-Type, origin, authorization, accept, client-security-token, host, date, cookie, cookie2, authtoken, x-debug, x-powered-by");
        header("access-control-allow-methods: " . ALLOWED_HTTP_METHODS);
    }

    /**
     * Process HTTP request
     * output JSON data
     */
    public function run() {
        $method = app()->getRequestMethod();
        $this->outputCorrHeaders();
        if($method != 'options')    //OPTIONS method is used to request CORS headers, no need to further processing
        {
            try {

                //retrieve and check method
                $allowedMethods = explode(",", strtolower(ALLOWED_HTTP_METHODS));
                if(!in_array($method, $allowedMethods))
                    throw new MethodNotAllowedException("Method $method is not allowed");


                $path = explode("/", app()->request("path"), 2);
                $ep = $path[0];
                $args = isset($path[1]) ? $path[1] : "";
                $endpoint = $this->getEndpoint($ep);

                // check if endpoint implements this method
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

            //output response
            header("Content-Type: application/json; charset=UTF-8");
            $ret["debug"] = app()->getDebugLog();
            echo json_encode($ret);
        }
    }
}
