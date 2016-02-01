#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <curl/curl.h>
using namespace std;

const auto Blank = "\r                                        \r";

static size_t OnWrite(void* data, size_t size, size_t count, void* userData)
{
    auto stream = (ostream*)userData;
    size_t total = size * count;
    stream->write((const char*)data, (streamsize)total);
    return total;
}

static int OnProgress(
    void* /*userData*/,
    double totalToDownload,
    double downloaded,
    double /*totalToUpload*/,
    double /*uploaded*/)
{
    cout
        << Blank
        << (int)totalToDownload
        << " bytes : "
        << (int)downloaded
        << " downloaded";
    cout.flush();
    return 0;
}

static bool Download(
    const char* uri,
    ostream& stream,
    bool showProgress = false)
{
    auto c = curl_easy_init();
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, OnWrite);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &stream);

    if (showProgress)
    {
        curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(c, CURLOPT_PROGRESSFUNCTION, OnProgress);
        //curl_easy_setopt(c, CURLOPT_PROGRESSDATA, this);
    }
    else
    {
        curl_easy_setopt(c, CURLOPT_NOPROGRESS, 1);
    }
    //curl_easy_setopt(c, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(c, CURLOPT_URL, uri);

    auto result = curl_easy_perform(c);
    if (showProgress) cout << Blank, cout.flush();
    return result == 0;
}

int main(int argc, char** argv)
{
    (void)argc, (void)argv;

    stringstream ss;

    Download("flashair/command.cgi?op=100&DIR=/DCIM/100__TSB", ss);

    string line;
    string uri;
    string file;
    while (getline(ss, line))
    {
        size_t a = line.find(',');
        if (a == string::npos) continue;
        size_t b = line.find(',', a + 1);
        if (b == string::npos) continue;
        //cout.write(line.data() + a + 1, b - a - 1) << endl;

        file.assign(line.data() + a + 1, b - a - 1);

        uri = "flashair/DCIM/100__TSB/";
        uri += file;

        //cout << uri << endl;
        ofstream fout(file, ofstream::binary);

        if (fout)
        {
            cout << "Downloading " << uri << '\n';
            cout.flush();

            if (!Download(uri.data(), fout, true))
            {
                cout << "FAILED" << endl;
            }

            fout.close();
        }
    }

    return 0;
}
