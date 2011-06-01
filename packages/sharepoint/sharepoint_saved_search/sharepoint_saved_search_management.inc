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
* $Id: sharepoint_saved_search_management.inc,v 1.3 2009/01/09 04:07:04 hnguyen Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Sharepoint_Saved_Search_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.3 $
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
