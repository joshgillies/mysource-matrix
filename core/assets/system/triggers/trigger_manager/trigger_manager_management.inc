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
* $Id: trigger_manager_management.inc,v 1.5 2006/12/05 23:44:44 bcaldwell Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Trigger_Manager_Management
*
* @see Asset_Management
*
* @author	Andrei Railean <arailean@squiz.net>
* @author	Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Manager_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Trigger_Manager_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'enabled'	=> Array(
										'added'			=> '0.1',
										'type'			=> 'boolean',
										'default'		=> FALSE,
										'parameters'	=> Array(
															'allow_empty'	=> FALSE,
														   ),
										'description'	=> 'A flag which enables or disables the execution of trigger events',
									   ),
					  );

	}//end constructor


}//end class
?>
