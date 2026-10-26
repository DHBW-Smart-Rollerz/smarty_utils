import os
from collections.abc import Sequence


def include_directory(
    install_path: str, source_path: str, exclude: Sequence[str] = ()
) -> list[tuple[str, list[str]]]:
    """Generates a list of tuples to include a whole directory into the data_files list.

    The list of tuples represents the directory structure and files within the given
    path, excluding specified file extensions.

    Each tuple in the list contains:
        - The path to a directory found within the traversed path.
        - A list of filenames within that directory, excluding those with
          extensions specified in the 'exclude' list.

    Args:
        install_path: The base path where files will be installed.
        source_path: The root directory containing the files to be included.
        exclude: A list of file extensions to exclude (e.g., [".txt", ".py"]).
            Defaults to no exclusions.

    Returns:
        A list of tuples with the directory a list of files in that directory.
    """
    install_path = install_path.removesuffix(source_path)

    # os.path instead of pathlib on purpose: Path normalizes paths (e.g. drops "./"),
    # which would change the install paths of existing packages.

    # Go through every nested sub-directory
    data_files = []
    for dirpath, _dirnames, filenames in os.walk(source_path):
        # Filter files by the file extension
        files = [
            os.path.join(dirpath, file)  # noqa: PTH118
            for file in filenames
            if not any(file.endswith(file_extension) for file_extension in exclude)
        ]

        if len(files) >= 1:
            data_files.append((os.path.join(install_path, dirpath), files))  # noqa: PTH118

    return data_files
