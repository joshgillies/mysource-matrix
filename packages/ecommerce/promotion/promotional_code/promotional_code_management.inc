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
* $Id: promotional_code_management.inc,v 1.3 2013/09/14 01:43:20 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Promotional_Code_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Code_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'				=> '0.1',
											'type'				=> 'text',
											'default'			=> '',
											'description'		=> 'The full name of the promotional code',
											'is_admin'			=> FALSE,
											'is_contextable'	=> TRUE,
										   ),
						'code'			=> Array(
											'added'				=> '0.1',
											'type'				=> 'text',
											'default'			=> '',
											'description'		=> 'The promotional code string',
											'is_admin'			=> FALSE,
											'is_contextable'	=> FALSE,
										   ),
						'rules'			=> Array(
											'added'				=> '0.1',
											'type'				=> 'serialise',
											'default'			=> Array(),
											'description'		=> 'The promotional rules applied to this promotional code',
											'is_admin'			=> TRUE,
											'is_contextable'	=> FALSE,
										   ),
						'conditions'	=> Array(
											'added'				=> '0.1',
											'type'				=> 'serialise',
											'default'			=> Array(),
											'description'		=> 'The conditions applied to this promotional code',
											'is_admin'			=> TRUE,
											'is_contextable'	=> FALSE,
										   ),										   
					  );

	}//end constructor


}//end class

?>