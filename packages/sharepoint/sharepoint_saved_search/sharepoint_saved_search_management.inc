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
* $Id: sharepoint_saved_search_management.inc,v 1.6 2012/08/30 00:59:19 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Sharepoint_Saved_Search_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __sharepoint__
*/
class Sharepoint_Saved_Search_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Sharepoint_Saved_Search_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'url'					=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
												   ),
						'cached_result'			=> Array (
													'added'			=> '0.1',
													'type'			=> 'serialise',
													'default'		=> Array(),
											   		'parameters'	=> Array (
												   						'allow_empty'		=> FALSE,
												   					   ),
												   ),
						'search_logic'			=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> 'And',
											   		'parameters'	=> Array (
												   						'allow_empty'		=> FALSE,
												   					   ),
												   ),
						'row_limit'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> 30,
													'parameters'	=> Array (
																		'allow_empty'		=> TRUE,
																		'allow_negative'	=> FALSE,
																	   ),
												   ),
						'data_list'				=> Array (
													'added'			=> '0.1',
													'type'			=> 'text',
													'default'		=> '',
											   		'parameters'	=> Array (
												   						'allow_empty'		=> FALSE,
												   					   ),
												   ),
						'download_files'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'description'	=> 'Whether to download the files attached with documents or not',
													'parameters'    => Array(
																		'allow_empty'   => FALSE,
												   					   ),
												   ),
						'search_options'		=> Array(
													'added'         => '0.1',
													'type'          => 'serialise',
													'default'       => Array(),
													'description'	=> 'Search option for each Sharepoint list',
													'is_admin'      => FALSE,
													'parameters'    => Array(
																		'allow_empty'   => FALSE,
																	   ),
												   ),
                        'username'				=> Array(
		                                            'added'     => '0.1',
		                                            'type'      => 'text',
		                                            'default'   => '',
		                                           ),
                        'password'				=> Array(
		                                        	'added'     => '0.1',
		                                            'type'      => 'password',
		                                            'default'   => '',
		                                           ),

						'dynamic_vars'			=> Array(
													'added'		=> '0.2',
													'type'		=> 'serialise',
													'default'	=> Array (
																   ),
												   ),
						'parameter_map'			=> Array (
													'added'			=> '0.1',
													'type'			=> 'parameter_map',
													'default'		=> Array(),
													'parameters'	=> Array (
																		'parameters'	=> Array(),
																	   ),
												   ),
						'download_file'			=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'description'	=> 'Whether to serve file locally or via the SharePoint URL',
											   		'parameters'	=> Array (
												   						'allow_empty'		=> FALSE,
												   					   ),
												),

						// This array allows Sharepoint data source to map the dynamic parameters with trim options
						// Eg: if you want to set a dynamic parameter to pass the keyword option in
						// with parameter name 'search_term', then trim_options['keyword'] need to be set to 'search_term'

					  );

	}//end constructor


    /**
    * Gets a list of all files that the bodycopy wants web accessable
    *
    * @return array
    * @access private
    */
    function _getFileList()
    {
        return Array(
                'js/ajax.js',
               );

    }//end _getFileList()


}//end class

?>
