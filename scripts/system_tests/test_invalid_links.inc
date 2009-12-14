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
* $Id: test_invalid_links.inc,v 1.1.2.1 2009/12/14 23:11:00 bpearson Exp $
*
*/

/**
* Test_Invalid_Links
*
*	A test to run to find invalid matrix links in content areas
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1.2.1 $
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
					$count++;
					$error[] = 'A link to Asset #'.$assetid.' was found on Content Type #'.$content_type;
				} else {
					$am->forgetAsset($link_asset, TRUE);
				}//end if
			}//end foreach
			$am->forgetAsset($asset, TRUE);
		}//end foreach

		if (!$status && !empty($count)) {
			$messages[] = 'There are '.$count.' content types with problems found';
		}//end if

		return $status;

	}//end test()

}//end class
?>
