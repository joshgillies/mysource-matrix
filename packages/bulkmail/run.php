#!/usr/bin/php
<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: run.php,v 1.3 2006/12/07 05:55:55 bcaldwell Exp $
*
*/

/**
* Index File
*
* The one file through which everything runs
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayne Ong <rong@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/

ini_set('memory_limit', -1);
require_once dirname(__FILE__).'/bulk_mailer.inc';

$bulk_mailer =& new Bulk_Mailer();
$bulk_mailer->start();

?>
