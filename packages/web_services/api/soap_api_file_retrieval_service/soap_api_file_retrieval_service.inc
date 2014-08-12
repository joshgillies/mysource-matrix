<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api_file_retrieval_service.inc,v 1.5 2012/08/30 00:59:45 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/web_services/api/soap_api/soap_api.inc';

/**
* Soap_Api_File_Retrieval_Service
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Api_File_Retrieval_Service extends Soap_Api
{


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function __construct($assetid=0)
    {
        parent::__construct($assetid);

    }//end constructor


    /**
    * Returns a function list of the asset let the children handle this
    *
    * @return string
    * @access private
    * @see Asset::_getName()
    */
    public function getFunctionList()
    {
        return Array(
				'Download'				=> '1',
        		'Upload'				=> '1',
        		'GetFileInformation'	=> '1',
                'GetImageInformation'   => '1',
			   );

    }//end getFunctionList()


	/**
	* Description: This operation allow transfer of file with content base64 encoded
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		 'AssetID'	 => [The AssetID of the file to download],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function Download($request)
	{
		$request_info = (Array) $request;
		$file_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($request_info['AssetID']);
		$type_ancestors	= $file_asset->getTypeAncestors(FALSE);
		if (in_array('file', $type_ancestors) || $file_asset->type() == 'file') {
			// Now we do the stuff
			$file_info				= $file_asset->getExistingFile();
			$file_path				= $file_info['path'];
			$file_content			= file_get_contents($file_path);
			$file_content_base64	= base64_encode($file_content);

			$file_name		= $file_info['filename'];
			$file_type		= get_file_type($file_name);
			$file_size		= $file_info['size'];
			$file_modified	= $file_info['modified'];

			return Array (
					'FileName'			=> $file_name,
					'FileType'			=> $file_type,
					'FileSize'			=> $file_size,
					'LastModified'		=> date('d-m-Y H:i:s', $file_modified),
					'FileContentBase64'	=> $file_content_base64,
				   );
		} else {
			throw new SoapFault('Server', 'The Asset ID Provided Is Not Of A File Asset');
		}//end else

		$GLOBALS['SQ_SYSTEM']->am->includeAsset($link_info['type_code']);
		$object_name = str_replace(' ', '_', ucwords(str_replace('_', ' ', $link_info['type_code'])));
		$asset = new $object_name();
		$asset->setAttrValue('name', $link_info['name']);

		$link_info['asset']	= $parent_asset;
		$linkid = $asset->create($link_info);
		if ($linkid) {
			return Array (
					'CreateMessage' => Array (
										'message'	=> 'Asset was created successfully. Link id #'.$linkid,
									   ),
					'AssetObject'	=> Array (
										'object_xml'	=> 'walao xml',
									   ),
				   );
		}//end if

	}//end Download()


	/**
	* Description: This operation return the information about the file based on the assetid
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		 'AssetID'	 => [The AssetID of the file to get information for],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetFileInformation($request)
	{
		$request_info = (Array) $request;
		$file_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($request_info['AssetID']);
		$type_ancestors	= $file_asset->getTypeAncestors(FALSE);
		if (in_array('file', $type_ancestors) || $file_asset->type() == 'file') {
			$file_info		= $file_asset->getExistingFile();

			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$file_name		= $file_info['filename'];
			$file_type		= get_file_type($file_name);
			$file_size		= $file_info['size'];
			$file_modified	= $file_info['modified'];

			return Array (
					'FileName'		=> $file_name,
					'FileType'		=> $file_type,
					'FileSize'		=> $file_size,
					'LastModified'	=> date('d-m-Y H:i:s', $file_modified),
				   );
		} else {
			throw new SoapFault('Server', 'The Asset ID Provided Is Not Of A File Asset');
		}//end else

	}//end GetFileInformation()


    /**
    * Description: This operation return the information about the image on the assetid
    *
    * @param array  $request    The request information
    * <pre>
    * Array (
    *        'AssetID'   => [The AssetID of the image to get information for],
    *        )
    * </pre>
    *
    * @return void
    * @access public
    */
    function GetImageInformation($request)
    {
        $request_info = (Array) $request;

        $file_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($request_info['AssetID']);
        if ($file_asset->type() === 'image') {
            $fileInfo   = $file_asset->getExistingFile();
            $image_info = getimagesize($fileInfo['path']);
            $mime_info  = $image_info['mime'];

            return Array (
                    'FileName'      => $file_asset->vars['name']['value'],
                    'FileTitle'     => $file_asset->vars['title']['value'],
                    'FileType'      => $file_asset->type(),
                    'FileSize'      => $file_asset->vars['size']['value'],
                    'MimeType'      => $mime_info,
                    'ImageWidth'    => $file_asset->vars['width']['value'],
                    'ImageHeight'   => $file_asset->vars['height']['value'],
                    'LastModified'  => date('d-m-Y H:i:s', $file_modified),
                   );
        } else {
            throw new SoapFault('Server', 'The Asset ID provided is not of a file asset');
        }//end else

    }//end GetImageInformation()

	/**
	* Description: This operation allow upload of file into an existing File Asset
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		'AssetID'			=> [The AssetID of the file asset to upload the file into],
	* 		'FileContentBase64'	=> [The content of the file base64 encoded],
	* 		'FileName'			=> [The name of the file being uploaded],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function Upload($request)
	{
		$request_info	= (Array) $request;
		$file_asset 	= $GLOBALS['SQ_SYSTEM']->am->getAsset($request_info['AssetID']);
		$file_content	= array_get_index($request_info, 'FileContentBase64', '');
		$file_name		= array_get_index($request_info, 'FileName', '');
		$type_ancestors	= $file_asset->getTypeAncestors(FALSE);
		if (in_array('file', $type_ancestors) || $file_asset->type() == 'file') {
			// Now we do the stuff
			$edit_fns	= $file_asset->getEditFns();
			if (!empty($file_content)) {
				//We are going to write to the Matrix Data tmp dir
				$tmp_file_name	= self::getRandomFilename($file_name);

				$destination_file	= SQ_DATA_PATH.'/temp/'.$tmp_file_name;
				while (file_exists($destination_file)) {
					sleep(1);
					$tmp_file_name	= self::getRandomFilename($file_name);
				}//end if
				file_put_contents($destination_file, base64_decode($file_content));
			}//end if

			// Construct the info array to pass to edit fns
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$file_type		= get_file_type($file_name);
			$info	= Array (
						'name'		=> $file_name,
						'type'		=> $file_type,
						'tmp_name'			=> $destination_file,
						'non_uploaded_file'	=> TRUE,
						'error'		=> 0,
						'size'		=> filesize($destination_file),
					  );

			//processFileUpload
			$prefix	= $file_asset->type().'_'.$file_asset->id;
			require_once SQ_INCLUDE_PATH.'/backend_outputter.inc';
			$o		= new Backend_Outputter();
			$GLOBALS['SQ_SYSTEM']->am->acquireLock($file_asset->id, 'attributes');

			$result	= $edit_fns->processFileUpload($file_asset, $o, $prefix, $info);
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($file_asset->id, 'attributes');

			// Remove the tmp file before we return
			if (file_exists($destination_file)) {
				unlink($destination_file);
			}//end if
			return Array (
					'UploadResult'		=> (string) $result,
				   );
		} else {
			throw new SoapFault('Server', 'The Asset ID Provided Is Not Of A File Asset');
		}//end else

	}//end Upload()


	/**
	* getRandomFileName get a random filename for the file being uploaded
	*
	* @param string $file_name	The original name of the file
	*
	* @return string
	* @access public
	*/
	public static function getRandomFilename($file_name)
	{
		$hash_file_name		= hash('md5', $file_name.time());
		$rand_position		= rand(0, strlen($hash_file_name)-6);
		$tmp_file_name		= substr($hash_file_name, $rand_position, 6);

		return $tmp_file_name;

	}//end getRandomFileName()


	/**
	* getComplexElements
	*
	* @return array
	* @access public
	*/
	public static function getComplexElements($func_list=Array())
	{
		$obj_optional	= self::getArgType('AssetObject', 0, 1);

		$complex_types	= Array (
							'Download'				=> Array (
														'Download'						=> Array (
																							'AssetID'			=> self::$string_non_optional,
																						   ),
														'DownloadResponse'				=> Array (
																							'FileName'			=> self::$string_optional,
																							'FileType'			=> self::$string_optional,
																							'FileSize'			=> self::$string_optional,
																							'LastModified'		=> self::$string_optional,
																							'FileContentBase64'	=> self::$string_optional,
																						   ),
													   ),
							'GetFileInformation'	=> Array (
														'GetFileInformation'			=> Array (
																							'AssetID'			=> self::$string_non_optional,
																						   ),
														'GetFileInformationResponse'	=> Array (
																							'FileName'			=> self::$string_optional,
																							'FileType'			=> self::$string_optional,
																							'FileSize'			=> self::$string_optional,
																							'LastModified'		=> self::$string_optional,
																						   ),
													   ),
                            'GetImageInformation'    => Array (
                                                        'GetImageInformation'            => Array (
                                                                                            'AssetID'           => self::$string_non_optional,
                                                                                           ),
                                                        'GetImageInformationResponse'    => Array (
                                                                                            'FileName'          => self::$string_optional,
                                                                                            'FileTitle'         => self::$string_optional,
                                                                                            'FileType'          => self::$string_optional,
                                                                                            'FileSize'          => self::$string_optional,
                                                                                            'MimeType'          => self::$string_optional,
                                                                                            'ImageWidth'        => self::$string_optional,
                                                                                            'ImageHeight'       => self::$string_optional,
                                                                                            'LastModified'      => self::$string_optional,
                                                                                           ),
                                                       ),
							'Upload'				=> Array (
													 	'Upload'						=> Array (
																							'AssetID'			=> self::$string_non_optional,
													   										'FileName'			=> self::$string_non_optional,
													   										'FileContentBase64'	=> self::$string_non_optional,
													   									   ),
														'UploadResponse'				=> Array (
														   									'UploadResult'		=> self::$string_optional,
													   									   ),


													   ),
						  );

		$complex_types_available = parent::getComplexElements($complex_types);

		return $complex_types_available;

	}//end getComplexElements()


	/**
	* getSimpleRestrictedTypes
	*
	* @return array
	* @access public
	*/
	public static function getSimpleRestrictedTypes($func_list=Array())
	{
		return Array();

	}//end getSimpleRestrictedTypes()


}//end class
?>
