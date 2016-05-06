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
* Git_Bridge_Management
*
* @see Asset_Management
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage git
*/
class Git_Bridge_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'		=> Array(
							  			'added'			=> '0.1',
								  		'type'			=> 'text',
								  		'default'		=> '',
								  		'description'	=> 'The name of the bridge',
								  		'update_fn'		=> '',
								  		'is_admin'		=> '0',
							  	   ),

						'clone_url'	=> Array(
									'added'			=> '0.1',
									'type'			=> 'text',
									'default'		=> '',
									'description'	=> 'SSH clone URL',
								   ),

						'sub_dirs'	=> Array(
									'added'			=> '0.1',
									'type'			=> 'text',
									'default'		=> '',
									'description'	=> 'Comma seperated sub directories',
								   ),

						'username'	=> Array(
									'added'			=> '0.1',
									'type'			=> 'text',
									'default'		=> '',
									'description'	=> 'Username to access private respository when using HTTPS Git URL',
								   ),

						'password'	=> Array(
									'added'			=> '0.1',
									'type'			=> 'password',
									'default'		=> '',
									'description'	=> 'Password to access private respository when using HTTPS Git URL',
								   ),

						'secret_key' => Array(
									'added'			=> '0.1',
									'type'			=> 'password',
									'default'		=> '',
									'description'	=> 'Secret key set at the Git to access the webhook URL',
								   ),
					  );

	}//end constructor


}//end class

?>
