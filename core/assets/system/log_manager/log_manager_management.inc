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
* $Id: log_manager_management.inc,v 1.4 2006/12/06 04:40:08 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Log_Manager_Management
*
* @see Asset_Management
*
* @author	Andrei Railean <arailean@squiz.net>
* @author	Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Log_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Log_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'enabled'	=> Array(
										'added'			=> '0.1',
										'type'			=> 'boolean',
										'default'		=> TRUE,
										'parameters'	=> Array(
															'allow_empty'	=> FALSE,
														   ),
										'description'	=> 'A flag which enables or disables logging',
									   ),
						'logs'		=> Array(
										'added'			=> '0.1',
										'type'			=> 'serialise',
										'default'		=> Array(),
										'description'	=> 'Log metadata relating to rotations',
									   ),
					  );


	}//end constructor


}//end class

?>
