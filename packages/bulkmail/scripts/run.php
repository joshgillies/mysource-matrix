#!/usr/bin/php
<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: run.php,v 1.3 2008/12/17 23:17:59 bshkara Exp $
*
*/

/**
* Run File 
*
* The one file through which everything runs
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/

ini_set('memory_limit', -1);
ini_set('error_reporting', E_ALL & (~E_STRICT));
require_once dirname(__FILE__).'/../bulk_mailer.inc';

$bulk_mailer = new Bulk_Mailer();
$bulk_mailer->start();

?>
