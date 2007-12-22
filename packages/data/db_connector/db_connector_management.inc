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
* $Id: db_connector_management.inc,v 1.2 2007/12/22 02:24:41 gsherwood Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Db_Connector
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.2 $
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
										'added'		=> '0.1',
										'type'		=> 'text',
										'default'	=> '',
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
