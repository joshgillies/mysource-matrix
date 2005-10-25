#!/usr/local/bin/php
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
* $Id: run.php,v 1.1.1.1 2005/10/25 04:53:43 ndvries Exp $
*
*/

/**
* Index File
*
* The one file through which everything runs
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayne Ong <rong@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix
*/

ini_set('memory_limit', -1);
require_once dirname(__FILE__).'/bulk_mailer.inc';

$bulk_mailer =& new Bulk_Mailer();
$bulk_mailer->start();

?>
