<?php
/**
 * FileSystem Lib.
 *
 * @version    4.0.0
 * @package    MySource4
 * @subpackage Libs
 * @author     Squiz Pty Ltd <mysource4@squiz.net>
 * @copyright  2006-2007 Squiz Pty Ltd (ABN 77 084 670 600)
 * @license    http://matrix.squiz.net/licence Squiz.Net Open Source Licence
 */

/**
 * Library for general file system operations.
 *
 * Methods for adding, deleting and modifying directories and files should
 * reside here.
 *
 * @since 4.0.0
 */
class FileSystem
{

    /**
     * The number of files within a hash directory.
     *
     * @var int
     */
    const HASH_DIR_FILE_COUNT = 10000;

    /**
     * Different units of measurement for sizes of files.
     *
     * @var string
     */
    const FILE_SIZE_BYTE     = 'B';
    const FILE_SIZE_KILOBYTE = 'kB';
    const FILE_SIZE_MEGABYTE = 'MB';
    const FILE_SIZE_GIGABYTE = 'GB';


    /**
     * Removes a directory from the file system.
     *
     * Enters subdirectories and removes all content, and deletes the parent
     * directory if specified.
     *
     * @param string  $path       The path of the directory to delete.
     * @param boolean $deleteRoot Whether to delete the parent directory.
     * @param array   $omitFiles  Array of file names that lie in the root
     *                            directory that should not be deleted. Note,
     *                            the $deleteRoot param should be false if this
     *                            is not empty.
     *
     * @since  4.0.0
     * @return boolean
     * @throws RuntimeException When the environment prevents the directory from
     *                          being succesfully deleted.
     */
    public static function clearDirectory($path, $deleteRoot=FALSE, array $omitFiles=array())
    {
        if (file_exists($path) === FALSE) {
            return FALSE;
        }

        $dir = new DirectoryIterator($path);
        while ($dir->valid() === TRUE) {
            if ($dir->isDot() === TRUE) {
                // This is '.' or '..'. Not interested.
                $dir->next();
                continue;
            }

            if ($dir->isDir() === TRUE) {
                // If $deleteRoot is FALSE, then we don't want to delete
                // .svn entries, as with directories like the oven directory,
                // this will break updates on parent directories.
                if ($deleteRoot === FALSE) {
                    if ($dir->getFileName() === '.svn') {
                        $dir->next();
                        continue;
                    }
                }

                // Recursively call this method to clear nested directories.
                if (self::clearDirectory($dir->getPathName(), FALSE) === FALSE) {
                    return FALSE;
                }

                // Directory is now clear, so we can remove the directory.
                if (rmdir($dir->getPathName()) === FALSE) {
                    // Unable to delete the directory. Throw Exception, as the
                    // operation will ultimately fail.
                    $error = 'Unable to delete directory: '.$dir->getPathName;
                    throw new RuntimeException($error);
                }

                $dir->next();
                continue;
            }//end if $dir->isDir()

            // We have a file or symbolic link, we need to unlink it.
            if (in_array($dir->getFileName(), $omitFiles) === FALSE) {
                if (unlink($dir->getPathName()) === FALSE) {
                    // Problem deleting the file, throw exception.
                    $error = 'Unable to delete file: '.$dir->getPathName;
                    throw new RuntimeException($error);
                }
            }

            $dir->next();
        }//end while

        if ($deleteRoot === TRUE) {
            if (file_exists($path) === TRUE) {
                if (rmdir($path) === FALSE) {
                    $error = 'Unable to delete directory: '.$dir->getPathName;
                    throw new RuntimeException($error);
                }
            }
        }

        return TRUE;

    }//end clearDirectory()


    /**
     * Lists the contents of a directory.
     *
     * Allows the specifying of file extensions, and whether or not to include
     * full paths in the resulting array, or just filenames. Returns an array
     * of file names.
     *
     * @param string  $path       The path of the directory to list.
     * @param array   $extensions An array of extensions (with the leading dots)
     *                            of files to return.
     * @param boolean $nested     Include subdirectories in the listing.
     * @param boolean $fullPath   Include the full path, or just the filename.
     *
     * @since  4.0.0
     * @return array
     */
    public static function listDirectory($path, array $extensions=array(), $nested=TRUE, $fullPath=TRUE)
    {
        $files = array();
        if (file_exists($path) === FALSE) {
            return $files;
        }

        $dir = new DirectoryIterator($path);
        while ($dir->valid() === TRUE) {
            if ($dir->isDot() === TRUE) {
                // This is '.' or '..'. Not interested.
                $dir->next();
                continue;
            }

            if ($dir->isDir() === TRUE && $nested === TRUE) {
                if ($dir->getFileName() === '.svn') {
                    $dir->next();
                    continue;
                }

                // Recursively call this method to list nested directories.
                $nestedDirectory = self::listDirectory($dir->getPathName(), $extensions, $nested, $fullPath);
                // Merge the files from the subdirectory with ours.
                $files = array_merge($files, $nestedDirectory);
                $dir->next();
                continue;
            }

            // We have a file, so we need to determine if it fits the specified
            // criteria.
            if ($dir->isFile() === TRUE) {

                $addFile  = FALSE;
                $filename = $dir->getFilename();

                // Skip hidden files.
                if ($filename{0} !== '.') {

                    if (empty($extensions) === TRUE) {
                        // No extensions, every file should be added.
                        $addFile = TRUE;
                    } else {
                        // Determine if the file has the right extension.
                        $fileName = $dir->getFileName();
                        $dotPos   = strrpos($fileName, '.');
                        $fileExt  = substr($fileName, $dotPos);
                        if (in_array($fileExt, $extensions) === TRUE) {
                            // Correct extension. Set our flag so it's added below.
                            $addFile = TRUE;
                        }
                    }

                    if ($addFile === TRUE) {
                        // File fits our criteria, now we only need to work out
                        // which details to add to our array.
                        if ($fullPath === TRUE) {
                            $files[] = $dir->getPathName();
                        } else {
                            $files[] = $dir->getFileName();
                        }
                    }
                }//end if

            }//end if isFile()

            $dir->next();
        }//end while valid()

        return $files;

    }//end listDirectory()


    /**
     * Recursively finds directories with the specified name.
     *
     * Returns the absolute paths to the directories, starting from the specified
     * path.
     *
     * @param string $path The path of the directory to start at.
     * @param string $name The name of the directories to find.
     *
     * @since  4.0.0
     * @return array
     */
    public static function findDirectories($path, $name)
    {
        $dir = new RecursiveDirectoryIterator($path);

        $paths = array();
        foreach ($dir as $file) {
            if ($file->isDir() === TRUE) {
                if ($file->getFilename() === $name) {
                    $paths[] = $file->getPathname();
                } else {
                    $paths = array_merge($paths, self::findDirectories($file->getPathName(), $name));
                }
            }
        }

        return $paths;

    }//end findDirectories()


    /**
     * Returns the relative hash directory for the specified id.
     *
     * @param integer $id The positive integer to return the hash dir for.
     *
     * @since  4.0.0
     * @return string
     * @throws InvalidArgumentException When a negative $id is passed.
     */
    public static function getHashDir($id)
    {
        $id = (int) $id;
        if ($id <= 0) {
            throw new InvalidArgumentException('$id must be positive');
        }

        $endId   = (ceil(($id / self::HASH_DIR_FILE_COUNT)) * self::HASH_DIR_FILE_COUNT);
        $startId = ($endId - self::HASH_DIR_FILE_COUNT + 1);

        return $startId.'-'.$endId;

    }//end getHashDir()


    /**
     * Return the size of a file with human readable units appended.
     *
     * @param string $path The path to the file.
     * @param string $unit Units to display file size with (see constants).
     *
     * @since  4.0.0
     * @return string
     */
    public static function readableSize($path, $unit=NULL)
    {
        $size = @filesize($path);
        if ($size === FALSE) {
            return FALSE;
        }

        $units = array(
                  self::FILE_SIZE_BYTE,
                  self::FILE_SIZE_KILOBYTE,
                  self::FILE_SIZE_MEGABYTE,
                  self::FILE_SIZE_GIGABYTE,
                 );
        $maxUnit = (count($units) - 1);

        // Accept units as a parameter, maybe...
        $unit = array_search($unit, $units);
        if ($unit === NULL || $unit === FALSE) {
            $unit = $maxUnit;
        }

        $factor = 0;
        while ($unit !== $factor && $size >= 1000 && $factor < $maxUnit) {
            $size = ($size / 1000);
            $factor++;
        }

        return sprintf('%01.2f%s', $size, $units[$factor]);

    }//end readableSize()


    /**
     * Basename for unix and Windows.
     *
     * @param string $path Path for a file.
     *
     * @since  4.0.0
     * @return string
     */
    public static function getBaseName($path)
    {
        $fileName = preg_replace( '/^.+[\\\\\\/]/', '', $path);
        return $fileName;

    }//end getBaseName()


}//end class

?>
