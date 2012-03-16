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
* $Id: test_invalid_links.inc,v 1.3 2012/03/16 05:19:48 akarelia Exp $
*
*/

/**
* Test_Invalid_Links
*
*	A test to run to find invalid matrix links in content areas
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
class Test_Invalid_Links
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'Invalid Matrix Links Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to check for invalid Matrix links in content areas';

	}//end getDescription()


	/**
	 * The test to run
	 *
	 * @param array	$messages	The variable to 'chuck'	all the messages
	 * @param array	$errors		The variable to 'chuck'	all the verbose messages
	 *
	 * @return boolean|int
	 * @access public
	 */
	public static function test(&$messages, &$errors)
	{
		$status = TRUE;
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$content_types = $am->getTypeAssetids('content_type', FALSE);
		$pattern = '/\\.\\/\\?a=([0-9]+)/';
		$count = 0;

		foreach ($content_types as $content_type) {
			$error_in_content_type = FALSE;
			$asset = $am->getAsset($content_type, '', TRUE);
			if (is_null($asset)) {
				$status = FALSE;
				$count++;
				$errors[] = 'Content Type #'.$content_type.' could not be loaded';
			}//end if

			$html = $asset->attr('html');
			preg_match_all($pattern, $html, $assetids);
			foreach ($assetids[1] as $assetid) {
				$link_asset = $am->getAsset($assetid, '', TRUE);
				if (is_null($link_asset)) {
					$status = FALSE;
					if (!$error_in_content_type) $count++;
					$error_in_content_type = TRUE;
					$errors[] = 'A link to Asset #'.$assetid.' was found on Content Type #'.$content_type;
				} else {
					$am->forgetAsset($link_asset, TRUE);
				}//end if
			}//end foreach
			$am->forgetAsset($asset, TRUE);
		}//end foreach

		$errors = array_unique($errors);
		if (!$status && !empty($count)) {
			$messages[] = 'There are '.$count.' content types with problems found';
		}//end if

		return $status;

	}//end test()

}//end class
?>
