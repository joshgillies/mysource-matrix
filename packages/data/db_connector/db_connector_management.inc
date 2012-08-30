<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: db_connector_management.inc,v 1.6 2012/08/30 00:58:16 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Db_Connector
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Db_Connector_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Db_Connector_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'dsn'		=> Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'name'		=> Array(
										'added'			=> '0.1',
										'type'			=> 'text',
										'default'		=> '',
										'is_admin'		=> FALSE,
										'is_contextable'=> TRUE,
									   ),
						'database'	=> Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'host'		=> Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'user'		=> Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '',
									   ),
						'password'	=> Array(
										'added'		=> '0.1',
										'type'		=> 'password',
										'default'	=> '',
									   ),
						'db_type'	=> Array(
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> 'postgres',
									   ),
					  );

	}//end constructor


}//end class

?>
