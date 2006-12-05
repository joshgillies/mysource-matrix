<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: connectivity.php,v 1.2 2006/12/05 05:05:25 bcaldwell Exp $
*
*/

require_once 'HTTP/Client.php';

/**
* Page to test remote connectivity
*
* Purpose
*     Check if a remote page exists (returns 200 OK)
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.2 $
*/


$Fetch_URL = &new Net_URL($_REQUEST['connect_url']);
$url = $Fetch_URL->getURL();

$request_parameters['timeout'] = 5;
$HTTP_Client =& new HTTP_Client($request_parameters);
$HTTP_Client->setMaxRedirects(5);

$result = $HTTP_Client->head($url);

if (!PEAR::isError($result)) {
	echo ($result == 200) ? 1 : 0;
} else {
	echo 0;
}
