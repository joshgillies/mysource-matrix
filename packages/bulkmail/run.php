#!/usr/bin/php
<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: run.php,v 1.2 2005/11/30 04:27:01 ndvries Exp $
*
*/

/**
* Index File
*
* The one file through which everything runs
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayne Ong <rong@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/

ini_set('memory_limit', -1);
require_once dirname(__FILE__).'/bulk_mailer.inc';

$bulk_mailer =& new Bulk_Mailer();
$bulk_mailer->start();

?>
