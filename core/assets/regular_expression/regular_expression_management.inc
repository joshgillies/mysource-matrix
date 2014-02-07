<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Regular_Expression_Management
*
* @see Asset_Management
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Regular_Expression_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager handling this asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'	=> Array(
									'added'			=> '0.1',
									'type'			=> 'text',
									'default'		=> '',
									'is_admin'		=> FALSE,
									'is_contextable'=> TRUE,
								   ),
						'regex_data' => Array(
									'added'			=> '0.1',
									'type'			=> 'serialise',
									'default'		=> Array(),
									'is_admin'		=> FALSE,
									'is_contextable'=> TRUE,
								   ),
						'limit'		=> Array(
									'added'			=> '0.1',
									'type'			=> 'int',
									'default'		=> '',
									'is_admin'		=> FALSE,
									'is_contextable'=> TRUE,
								   ),
					  );

	}//end constructor


}//end class
?>
