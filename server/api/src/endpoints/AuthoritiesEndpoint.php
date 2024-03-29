<?php

/**
 * Tabloo - open source bus stop information display
 * 
 * List of authorities
 * No auth
 * 
 * Handles GET request
 * Returns list of authorities
 * 
 * @author ilja.tihhanovski@gmail.com
 * 
 */


class AuthoritiesEndpoint extends RestEndpoint {

    public function get(string $path) {

        $sel = app()->db()->prepare("select distinct authority from stops order by authority");
        $sel->execute();
        $ret = $sel->fetchAll(PDO::FETCH_COLUMN, "authority");
        if(!$ret)
            throw new NotFoundException("Authorities not found");
        return $ret;
    }
}